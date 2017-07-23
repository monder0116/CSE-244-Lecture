
#ifndef QRDECOM_H
#define QRDECOM_H
#include <stdlib.h>
#include <math.h>
#define MaxMatrixSize 50
typedef struct {
	int m, n;
	double ** v;
} mat_t, *mat;
 mat matrix_new(int m, int n);
void matrix_delete(mat m);
void matrix_transpose(mat m);
void takeinverse(double array[][MaxMatrixSize], int n);
mat matrix_copy(double a[][MaxMatrixSize], int m, int n);
mat matrix_mul(mat x, mat y);

mat matrix_minor(mat x, int d);
double *vmadd(double a[], double b[], double s, double c[], int n);
mat vmul(double v[], int n);
double vnorm(double x[], int n);
double* vdiv(double x[], double d, double y[], int n);
double* mcol(mat m, double *v, int c);
void matrix_show(mat m);
void householder(mat m, mat *R, mat *Q);
void  QRSolver(double A[][MaxMatrixSize],double B[MaxMatrixSize],double result[MaxMatrixSize],int rowNum,int colNum);


 #endif 
 