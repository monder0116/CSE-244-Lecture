#ifndef MATRIXCAL
#define MATRIXCAL
#include <stdio.h>


#ifndef MATRIXMAXSIZE
#define MATRIXMAXSIZE 25
#endif
float determinant(float a[25][25], float k);
void cofactor(float num[25][25], float f);
void ConvolutionCalculate(float in[][25],float out[][25],int rows,int cols);
void CalculateShiftedMatrix(float arr[][25],float shiftedMatrix[][25],int matrixSize);
void calculateInverse(float a[][25],int n);
void transpose(float num[25][25], float fac[25][25], float r);
#endif