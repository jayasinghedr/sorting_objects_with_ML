#include "matrices.h"
#include <math.h>

//TCS3200 pins wiring to Arduino
#define S0 5
#define S1 6
#define S2 7
#define S3 8
#define sensorOut 3

//HCSR04 pins wiring to Arduino
#define echoPin 2 
#define trigPin 4 

//Switches
#define TRAIN_SW  A0
#define GOOD_SW   A1
#define BAD_SW    A2
#define CHECK_SW  A3

//LEDs
#define TRAIN_LED 10
#define GOOD_LED  11
#define BAD_LED   12
#define IDLE_LED  13

// defining array sizes
#define  TRAIN_R     6  // TRAIN_R = Sample size
#define  TRAIN_C     4
#define  OUT_R       1
#define  OUT_C       6 // OUT_C = Sample size
#define  WEIGHTS_R   1
#define  WEIGHTS_C   4

#define  ITERATIONS 100

// defines variables
long duration; // variable for the duration of sound wave travel
double distance; // variable for the distance measurement

// Stores frequency read by the photodiodes
double redFrequency = 0;
double greenFrequency = 0;
double blueFrequency = 0;

// Stores the red. green and blue colors
double redColor = 0;
double greenColor = 0;
double blueColor = 0;

double trainDataArr[VAR][VAR];
double trainOutArr[1][VAR];

double weights[1][VAR] = {{0, 0, 0, 0}};
double bias = 0;
double dataArr[1][VAR];
double dataArrT[4][VAR];
double result[1][VAR];

double output = 0;

int checkObject = 0;
int trainMode = 0;
int goodItem = 0;
int badItem = 0;
int state = 0;  //state = 0 -> idle; state = 2 -> train; state = 3 -> calculating
int trainSetCount = 0;

void takeReading()
{
  // RED
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  redFrequency = pulseIn(sensorOut, LOW);
  redFrequency = constrain(redFrequency, 30, 150);
  redColor = map(redFrequency, 59, 223, 255,0);
  dataArr[0][0] = redColor;
  delay(10);
  
  // GREEN
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  greenFrequency = pulseIn(sensorOut, LOW);
  greenFrequency = constrain(greenFrequency, 30, 150);
  greenColor = map(greenFrequency, 90, 180, 255, 0); 
  dataArr[0][1] = greenColor;
  delay(10);
 
  // BLUE
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  blueFrequency = pulseIn(sensorOut, LOW);
  blueFrequency = constrain(blueFrequency, 30, 60);
  blueColor = map(blueFrequency, 30, 100, 255, 0);
  dataArr[0][2] = blueColor;
  delay(10);

  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 * 10/ 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  dataArr[0][3] = distance;
}

//sigmoid fuction
double sigFunc( double mat[][VAR],
                double result[][VAR], 
                int r, int c) 
{
  for (int i = 0; i < r; ++i)
    for (int j = 0; j < c; ++j) {
      result[i][j] = 1/(1 + exp(-mat[i][j]));
    } 
}

double logisticRegression(double trainD[][VAR], int tR, int tC,
                          double outputD[][VAR], int oR, int oC,
                          double weights[][VAR], int wR, int wC, 
                          double bias)
{
  double temp[VAR][VAR];
  matT(trainD, temp, tR, tC);  //trainD.T
  double temp1[1][VAR];
  matMul(weights, temp, temp1, wR, wC, tC, tR); //weights @ trainD.T
  double temp2[1][VAR];
  matConstAdd(temp1, temp2, oR, oC, bias); //weights @ trainD.T + bias

  //Sigmoid Function
  double s[oR][VAR];
  sigFunc(temp2, s, oR, oC);
  
  // gradient descent
  double dW[wR][VAR];
  double dB;
  double m = oC;
  double alpha = 0.0005; // learn rate

  //computing dW
  matSub(s, outputD, s, oR, oC);
  matMul(s, trainD, dW, oR, oC, tR, tC);
  matConstMul(dW, dW, wR, wC, (alpha/m));

  //computing W with gradient decent
  matSub(weights, dW, weights, wR, wC);

  //computing dB
  dB = (alpha/m)*matSum(s, oR, oC);
  bias = bias - dB;
  return bias;
}


void setup() {
  // Setting the outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  pinMode(TRAIN_LED,  OUTPUT);
  pinMode(GOOD_LED,   OUTPUT);
  pinMode(BAD_LED,    OUTPUT);
  pinMode(IDLE_LED,   OUTPUT);
  
  // Setting the inputs
  pinMode(sensorOut, INPUT);

  pinMode(TRAIN_SW, INPUT);
  pinMode(GOOD_SW,  INPUT);
  pinMode(BAD_SW,   INPUT);
  pinMode(CHECK_SW, INPUT);
  
  
  // Setting frequency scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  
  // Begins serial communication
  Serial.begin(9600);
}

void loop() {
  checkObject = digitalRead(CHECK_SW);
  trainMode = digitalRead(TRAIN_SW);
  goodItem = digitalRead(GOOD_SW);
  badItem = digitalRead(BAD_SW);

  if (state == 0){
    digitalWrite(IDLE_LED, HIGH);
  }
  
  if (trainMode == HIGH){
    // press the TRAIN_SW to get in and out of Training mode
    // once out of traning mode parameters will be reset (e.g. trainSetCount)
    delay(300);
    if (state == 2) {
      //exit train mode
      digitalWrite(TRAIN_LED, LOW);
      state = 0;
      trainSetCount = 0;
    } else{
      Serial.println("Training Mode");
      digitalWrite(IDLE_LED, LOW);
      digitalWrite(TRAIN_LED, HIGH);
      state = 2;
    }
  }

  
  if ((trainSetCount == VAR)&& (state == 2)){
    state = 3;
    trainSetCount = 0;
  }

  if (state == 3) {
    //end the training mode and computes weights
    digitalWrite(IDLE_LED, HIGH);
    digitalWrite(TRAIN_LED, HIGH);
    for (int i = 0; i < ITERATIONS; ++i){
      bias = logisticRegression(trainDataArr, TRAIN_R, TRAIN_C, trainOutArr, OUT_R, OUT_C, weights, WEIGHTS_R, WEIGHTS_C, bias);
    }

    
    Serial.print("Weights = (");
    Serial.print(weights[0][0], 5);
    Serial.print("  ");
    Serial.print(weights[0][1], 5);
    Serial.print("  ");
    Serial.print(weights[0][2], 5);
    Serial.print("  ");
    Serial.print(weights[0][3], 5);
    Serial.print(")  bias = ");
    Serial.println(bias, 5);
    delay(500);
    digitalWrite(IDLE_LED, LOW);
    digitalWrite(TRAIN_LED, LOW);
    state = 0;
  }

  if (checkObject == HIGH){
    delay(200);
    
    if ((state == 2)) {
      //checking objects in training mode
      digitalWrite(IDLE_LED, LOW);
      
      for (int i = 0; i <= 5; ++i){
        //wait until the values settle
        takeReading();
        delay(200);
      }
      //add train values to trainDataArr
      trainDataArr[trainSetCount][0] = dataArr[0][0];
      trainDataArr[trainSetCount][1] = dataArr[0][1];
      trainDataArr[trainSetCount][2] = dataArr[0][2];
      trainDataArr[trainSetCount][3] = dataArr[0][3];
      Serial.print(" R = ");
      Serial.print(trainDataArr[trainSetCount][0]);
      Serial.print(" G = ");
      Serial.print(trainDataArr[trainSetCount][1]);
      Serial.print(" B = ");
      Serial.print(trainDataArr[trainSetCount][2]);
      Serial.print(" Distance = ");
      Serial.println(trainDataArr[trainSetCount][3]);
      trainSetCount++;
    }
    
    else{
      //checking objects in evaluation mode
      digitalWrite(IDLE_LED, LOW);
      Serial.println("Checking Object...");
      
      for (int i = 0; i <= 5; ++i){
        //wait until the values settle
        takeReading();
        delay(200);
      }
      
      matT(dataArr, dataArrT, 1, 4);
      matMul(weights, dataArrT, result, 1, 4, 4, 1);
      output = 1/(1+exp(-1*(result[0][0] + bias)));
      
      Serial.print(" R = ");
      Serial.print(dataArr[0][0]);
      Serial.print(" G = ");
      Serial.print(dataArr[0][1]);
      Serial.print(" B = ");
      Serial.print(dataArr[0][2]);
      Serial.print(" Distance = ");
      Serial.println(dataArr[0][3]);
      Serial.print("Probability = ");
      Serial.println(output, 4);
  
      if (output >= 0.90) {
        digitalWrite(GOOD_LED, HIGH);
        Serial.println("Good");
      } else {
        digitalWrite(BAD_LED, HIGH);
        Serial.println("Bad");
      }
      delay(5000);
      digitalWrite(GOOD_LED, LOW);
      digitalWrite(BAD_LED, LOW);
      state = 0;
    }
  } 

  if ((goodItem == HIGH) && (state == 2)){
    delay(200);
    trainOutArr[0][trainSetCount] = 1;
    Serial.print(trainSetCount);
    Serial.println(" = Good");
  }
   
  if (badItem == HIGH && (state == 2)){
    delay(200);
    trainOutArr[0][trainSetCount] = 0;
    Serial.print(trainSetCount);
    Serial.println(" = Bad");
  }
}
