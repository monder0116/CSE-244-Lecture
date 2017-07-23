#include "matrixProg.h"

void createMatrix(double Aarray[][MaxMatrixSize],double Barray[MaxMatrixSize],int rownum,int colnum){
	int i,j;


	srand(time(NULL));
	for ( i = 0; i < rownum; ++i)
	{   
		for ( j = 0; j < colnum; ++j)
		{
			
			double temp=rand() % 13 ;
			Aarray[i][j]=temp;

		}
	}
	srand(time(NULL));
	for (i = 0; i < rownum; ++i)
	{
		
		double temp=rand()%11;
		Barray[i]=temp;
	}

}
double 	standardDeviation(double xresult[MaxMatrixSize],int size){

	double xavarage=0;
	double stotal=0;

	int i;
	for ( i = 0; i < size; ++i)
	{
		xavarage=xresult[i];
	}	
	xavarage/=size;

	for (i = 0; i < size; ++i)
	{
		stotal+=pow((xresult[i]*xresult[i] - xavarage),2);

	}
	stotal/=size-1;
	double result=sqrt(stotal);

	return result;

}
void matrixMultiplication(double result[][MaxMatrixSize],double a[][MaxMatrixSize],double b[][MaxMatrixSize],int r1,int c1,int r2,int c2){
	    // Initializing all elements of result matrix to 0
	int i,j,k;
	for(i=0; i<r1; ++i)
		for(j=0; j<c2; ++j)
		{
			result[i][j] = 0;
		}


		if(c1!=r2)
			return;

    // Multiplying matrices a and b and
    // storing result in result matrix
		for(i=0; i<r1; ++i)
			for(j=0; j<c2; ++j)
				for(k=0; k<c1; ++k)
				{
					result[i][j]+=a[i][k]*b[k][j];
				}
			}
			void takeinverse(double array[50][50], int n)
			{

				double ratio,a;
				int i, j, k;

				for(i = 0; i < n; i++){
					for(j = n; j < 2*n; j++){
						if(i==(j-n))
							array[i][j] = 1.0;
						else
							array[i][j] = 0.0;
					}
				}
				for(i = 0; i < n; i++){
					for(j = 0; j < n; j++){
						if(i!=j){
							ratio = array[j][i]/array[i][i];
							for(k = 0; k < 2*n; k++){
								array[j][k] -= ratio * array[i][k];
							}
						}
					}
				}
				for(i = 0; i < n; i++){
					a = array[i][i];
					for(j = 0; j < 2*n; j++){

						array[i][j] /= a;
					}
				}

				for(i = 0; i < n; i++){
					for(j = 0; j < n; j++){
						array[i][j] = array[i][j+n];
						array[i][j+n] = 0;
					}
				}

			}
			void takeTranspose(double arr[][50],double newarr[50][50],int r,int c){

				int i,j;
				for ( i = 0; i < r; ++i)
				{
					for ( j = 0; j < c; ++j)
					{
						newarr[j][i]=arr[i][j];

					}
				}

			}


			void pseudoinverseCalculator(double A[][MaxMatrixSize], double B[MaxMatrixSize],double results[MaxMatrixSize], int row ,int col){
				double atrans[MaxMatrixSize][MaxMatrixSize],mult2[MaxMatrixSize][MaxMatrixSize],mult3[MaxMatrixSize][MaxMatrixSize],aamult[MaxMatrixSize][MaxMatrixSize],tempB[MaxMatrixSize][MaxMatrixSize];
				int i,j;
				for (i = 0; i < row; ++i)
				{
					tempB[i][0]=B[i];
				}

				takeTranspose(A,atrans,row,col);
				matrixMultiplication(aamult,atrans,A,col,row,row,col);
				takeinverse(aamult,col);
				matrixMultiplication(mult2,aamult,atrans,col,col,col,row);
				matrixMultiplication(mult3,mult2,tempB,col,row,row,1);
				for ( i = 0; i < col; ++i)
				{
					results[i]=mult3[i][0];

				}




			}
			double calculateError(double A[][MaxMatrixSize],double xresults[MaxMatrixSize],double B[MaxMatrixSize] ,int rowNum,int colNum){
				double ax[MaxMatrixSize][MaxMatrixSize],transposeError[MaxMatrixSize][MaxMatrixSize],errormatrix[MaxMatrixSize][MaxMatrixSize];
				int i,j;
				for ( i = 0; i < rowNum; ++i)
				{

					double sum=0;
					for ( j = 0; j < colNum; ++j)
					{
						sum+=A[i][j]*xresults[j];

					}
					ax[i][0]=sum;
				}

				for ( i = 0; i < rowNum; ++i)
				{
					errormatrix[i][0]=ax[i][0]-B[i];
					transposeError[0][i]=errormatrix[i][0];

				}

				double sum=0;

				for ( i = 0; i < rowNum; ++i)
				{
					sum+=transposeError[0][i]*errormatrix[i][0];
				}
				sum=sqrt(sum);
				return sum;
			}
/*
int main(){
	double A[50][50];
	double x[50];
	double B[50][50];

	A[0][0]=1;
	A[0][1]=2;
	A[0][2]=3;
	
A[1][0]=2;
	A[1][1]=-1;
	A[1][2]=1;
		
A[2][0]=3;
	A[2][1]=0;
	A[2][2]=-1;

	B[0][0]=9;
	B[1][0]=8;
	B[2][0]=3;
	
	int i;
	
	//fprintf(stderr, "%lf\n", calculateError(A,x,B,3,3));
	pseudoinverseCalculator(A,B,x,3,3);
	for ( i = 0; i < 3; ++i)
	{
		fprintf(stderr, "%lf\n", x[i]);
	}
	return 0;
}*/