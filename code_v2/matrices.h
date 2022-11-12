// header file for matrix computations
#define    VAR 6

void matMul(  double first[][VAR],
                double second[][VAR],
                double result[][VAR],
                int r1, int c1, int r2, int c2) {

   // Initializing elements of matrix mult to 0.
   for (int i = 0; i < r1; ++i) {
      for (int j = 0; j < c2; ++j) {
         result[i][j] = 0;
      }
   }

   // Multiplying first and second matrices and storing it in result
   for (int i = 0; i < r1; ++i) {
      for (int j = 0; j < c2; ++j) {
         for (int k = 0; k < c1; ++k) {
            result[i][j] += first[i][k] * second[k][j];
         }
      }
   }
}

void emptyMat(  double mat[][VAR], int r, int c)
{
  for (int i = 0; i < r; ++i)
      for (int j = 0; j < c; ++j) {
         mat[i][j] = 0;
      }
}


void matT(  double original[][VAR],
          double transpose[][VAR],
            int r, int c)
{
   // computing the transpose
   for (int i = 0; i < r; ++i)
      for (int j = 0; j < c; ++j) {
         transpose[j][i] = original[i][j];
      }
}

void matAdd(  double mat1[][VAR], 
                double mat2[][VAR],
                double result[][VAR],
                int r, int c  )
{
   for (int i = 0; i < r; ++i)
      for (int j = 0; j < c; ++j) {
         result[i][j] = mat1[i][j] + mat2[i][j];
      }
}

void matSub(  double mat1[][VAR], 
                double mat2[][VAR],
                double result[][VAR],
              int r, int c)
{
   for (int i = 0; i < r; ++i)
      for (int j = 0; j < c; ++j) {
         result[i][j] = mat1[i][j] - mat2[i][j];
      }
}



void matConstAdd( double mat[][VAR], 
          double result[][VAR],
          int r, int c, double bias)
{
  for (int i = 0; i < r; ++i)
      for (int j = 0; j < c; ++j) {
        result[i][j] = mat[i][j] + bias;
        }
}

void matConstMul( double mat[][VAR], 
          double result[][VAR],
          int r, int c, double m)
{
  for (int i = 0; i < r; ++i)
      for (int j = 0; j < c; ++j) {
        result[i][j] = mat[i][j]*m;
        }
}

double matSum(  double mat[][VAR],
          int r, int c)
{
  double total = 0;
  for (int i = 0; i < r; ++i)
      for (int j = 0; j < c; ++j) {
        total += mat[i][j];
        }
    return total;   
}

void matDisplay( double mat[][VAR], int r, int c)
{
  for (int i = 0; i < r; ++i)
    for (int j = 0; j < c; ++j) {
      printf("%10.8f  ", mat[i][j]);
      if (j == c - 1) {
        printf("\n");
      }
    }
}
