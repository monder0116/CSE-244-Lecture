#ifndef MATRIXPROG
#define MATRIXPROG
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#define MaxMatrixSize 50
typedef struct{
	int blockSolve;
	int blockVerify;
	double Aarray[MaxMatrixSize][MaxMatrixSize];
	double Barray[MaxMatrixSize];
	double QRresults[MaxMatrixSize];
	double SVDresults[MaxMatrixSize];
	double inverseResults[MaxMatrixSize];
	float averageTime;
	double errorQR;
	double errorSVD;
	double errorInverse;
	int rowNum;
	int colNum;
}RequestMatrixType;

void createMatrix(double Aarray[][MaxMatrixSize],double Barray[],int row,int col );
void takeTranspose(double arr[][MaxMatrixSize],double newarr[][MaxMatrixSize],int r,int c);
double 	standardDeviation(double xresult[MaxMatrixSize],int size);
void pseudoinverseCalculator(double A[][MaxMatrixSize], double B[MaxMatrixSize],double results[MaxMatrixSize], int row ,int col);
double calculateError(double A[][MaxMatrixSize],double xresults[MaxMatrixSize],double B[MaxMatrixSize] ,int rowNum,int colNum);

#endif
