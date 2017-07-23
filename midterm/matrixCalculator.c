
#include <stdio.h>
#include "matrixCalculator.h"
#include <math.h>
#include <time.h>

float determinant(float a[25][25], float k)

{

  float s = 1, det = 0, b[25][25];

  int i, j, m, n, c;

  if (k == 1)

  {

   return (a[0][0]);

 }

 else

 {

   det = 0;

   for (c = 0; c < k; c++)

   {

    m = 0;

    n = 0;

    for (i = 0;i < k; i++)

    {

      for (j = 0 ;j < k; j++)

      {

        b[i][j] = 0;

        if (i != 0 && j != c)

        {

         b[m][n] = a[i][j];

         if (n < (k - 2))

          n++;

        else

        {

         n = 0;

         m++;

       }

     }

   }

 }

 det = det + s * (a[0][c] * determinant(b, k - 1));

 s = -1 * s;

}

}



return (det);

}



void cofactor(float num[25][25], float f)

{

 float b[25][25], fac[25][25];

 int p, q, m, n, i, j;

 for (q = 0;q < f; q++)

 {

   for (p = 0;p < f; p++)

   {

     m = 0;

     n = 0;

     for (i = 0;i < f; i++)

     {

       for (j = 0;j < f; j++)

       {

        if (i != q && j != p)

        {

          b[m][n] = num[i][j];

          if (n < (f - 2))

           n++;

         else

         {

           n = 0;

           m++;

         }

       }

     }

   }

   fac[q][p] = pow(-1, q + p) * determinant(b, f - 1);

 }

}

transpose(num, fac, f);

}

/*Finding transpose of matrix*/ 

void transpose(float num[25][25], float fac[25][25], float r)

{

  int i, j;

  float b[25][25], inverse[25][25], d;



  for (i = 0;i < r; i++)

  {

   for (j = 0;j < r; j++)

   {

     b[i][j] = fac[j][i];

   }

 }

 d = determinant(num, r);

 for (i = 0;i < r; i++)

 {

   for (j = 0;j < r; j++)

   {

    inverse[i][j] = b[i][j] / d;
    num[i][j]=inverse[i][j] ;
    if(isinf(num[i][j])){
      num[i][j]=1;
    }
  }

}

}
void CalculateShiftedMatrix(float arr[][25],float shiftedMatrix[][25],int matrixSize){
  float matrix1[25][25];
  float matrix2[25][25];
  float matrix3[25][25];
  float matrix4[25][25];
  int i,j;

  int matrixNsize=matrixSize/2;
  for ( i = 0; i < matrixSize; ++i)
  {
    for ( j = 0; j < matrixSize; ++j)
    {
      if(i<matrixNsize && j<matrixNsize){

        matrix1[i][j]=arr[i][j];
      }
      else if(i>matrixNsize-1 && j<matrixNsize){
        matrix2[i-matrixNsize][j]=arr[i][j];

      }
      else if(i<matrixNsize && j>matrixNsize-1){
        matrix3[i][j-matrixNsize]=arr[i][j];

      }
      else{
        matrix4[i-matrixNsize][j-matrixNsize]=arr[i][j];
      }
    }
  }
  float mat1det=determinant(matrix1,matrixNsize);
  if(mat1det!=0){
    cofactor(matrix1,matrixNsize);
  }else{
    for ( i = 0; i < matrixNsize; ++i)
    {
      for ( j = 0; j < matrixNsize; ++j)
      {
        matrix1[i][j]=1;
      }

    }
  }
  float mat2det=determinant(matrix2,matrixNsize);
  if(mat2det!=0)
    cofactor(matrix2,matrixNsize);
  else{
    for ( i = 0; i < matrixNsize; ++i)
    {
     for ( j = 0; j < matrixNsize; ++j)
     {
      matrix2[i][j]=1;
    }
  }
}
float mat3det=determinant(matrix3,matrixNsize);
if(mat3det!=0)
  cofactor(matrix3,matrixNsize);
else{
  for ( i = 0; i < matrixNsize; ++i)
  {
    for ( j = 0; j < matrixNsize; ++j)
    {
      matrix3[i][j]=1;
    }
  }
}
float mat4det=determinant(matrix4,matrixNsize);
if(mat4det!=0)
  cofactor(matrix4,matrixNsize);
else{
  for ( i = 0; i < matrixNsize; ++i)
  {
    for ( j = 0; j < matrixNsize; ++j)
    {
      matrix4[i][j]=1;
    }
  }
}



for ( i = 0; i < matrixSize; ++i)
{
  for ( j = 0; j < matrixSize; ++j)
  {
    if(i<matrixNsize && j<matrixNsize){
      shiftedMatrix[i][j]=matrix1[i][j];
    }
    else if(i>matrixNsize-1 && j<matrixNsize){

      shiftedMatrix[i][j]=matrix2[i-matrixNsize][j];
    }
    else if(i<matrixNsize && j>matrixNsize-1){

      shiftedMatrix[i][j]=matrix3[i][j-matrixNsize];
    }
    else{
      shiftedMatrix[i][j]=matrix4[i-matrixNsize][j-matrixNsize];
    }
  }
}
for ( i = matrixSize; i < 25; ++i)
{
  for ( j = matrixSize; j < 25; ++j)
  {
    shiftedMatrix[i][j]=0;
  }
}


}


void ConvolutionCalculate(float in[][25],float out[][25],int rows,int cols){

  // find center position of kernel (half of kernel size)
  float kernel[3][3];
  int kCols=3;
  int kRows=3;
  int kCenterX = kCols / 2;
  int  kCenterY = kRows / 2;
  int i,j,m,n;
  for ( i = 0; i < rows; ++i)
  {
    for ( j = 0; j < cols; ++j)
    {
      out[i][j]=0;
    } 
  }

  for ( i = 0; i < 3; ++i)
  {
    for ( j = 0; j < 3; ++j)
    {

      if(i==j){
        kernel[i][j]=1;
      }
      else
        kernel[i][j]=0;
    }
  }

for(i=0; i < rows; ++i)              // rows
{
    for(j=0; j < cols; ++j)          // columns
    {
        for(m=0; m < kRows; ++m)     // kernel rows
        {
           int mm = kRows - 1 - m;      // row index of flipped kernel

            for(n=0; n < kCols; ++n) // kernel columns
            {
               int nn = kCols - 1 - n;  // column index of flipped kernel

                // index of input signal, used for checking boundary
               int ii = i + (m - kCenterY);
               int jj = j + (n - kCenterX);

                // ignore input samples which are out of bound
               if( ii >= 0 && ii < rows && jj >= 0 && jj < cols )
                out[i][j] += in[ii][jj] * kernel[mm][nn];
            }
          }
        }
      }



    }




/*Finding transpose of matrix*/ 
/*

    int main(){
      float arr[25][25];
      srand(time(NULL));
      int i,j;
      for ( i = 0; i < 10; ++i)
      {
        for (j = 0; j < 10; ++j)
        {
         arr[i][j]=rand()%15;
         fprintf(stderr, "%lf,",arr[i][j] );
       }
       fprintf(stderr, "\n");
     }


     for ( i = 10; i < 25; ++i)
     {
      for (j = 10; j < 25; ++j)
      {
       arr[i][j]=0;
     }
   }
   arr[0][0]=1;arr[0][1]=4;
   arr[1][0]=9;arr[1][1]=5;
   fprintf(stderr, "tersi---------------\n");

   cofactor(arr,2);


   return 0;
 }*/  