#include "stdio.h"
#include "stdlib.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "matrixCalculator.h"
#include "StructTypes.h"
#include <sys/stat.h>
#define MATRIXMAXSIZE 25
typedef struct{
	float arr[MATRIXMAXSIZE][MATRIXMAXSIZE];
	char type;
	double elapsedtime;
}MatrixArrayType;

static float matrix[25*25];
static int writeServerDes;
static char myfifoname[40];
static char mainfifoname[40];
static int readFifo;
static pid_t parentTimeServerpid;
static pid_t parentClintpid;
void killhander(int signo){
	if(signo==SIGINT && parentClintpid==getpid()){
		close(readFifo);
		close(writeServerDes);
		kill(0,SIGINT);
		while(wait(NULL)!=-1 || errno==EINTR);
		unlink(myfifoname);
	
		kill(parentTimeServerpid,SIGINT);
		fprintf(stderr, "Ctrl+C signal was taken\n");
		exit(0);
	}

	else{
		close(readFifo);
		close(writeServerDes);
		exit(0);
	}
}
int main(int argnum,char **args){

	
	if(argnum!=2){
		fprintf(stderr, "Usage=./seeWhat <mainpipename>");
		return 0;
	}
	if(strcmp(args[0],"./seeWhat")!=0){
		fprintf(stderr, "Usage=./seeWhat <mainpipename>");
		return 0;
	}																				/*Take mainfifo name and open the fifo for writing the																											own pid*/
		strcpy(mainfifoname,args[1])	;
	int signalstate=0;
	writeServerDes=open(mainfifoname,O_WRONLY);
	if(writeServerDes==-1){
		fprintf(stderr, "Firstly timeServer must opened \n");
		return -1;
	}
	int flag=0;
	int tempcell=0;
	int matrixSize=0;
	pid_t serverpid;
	int logcount=0;
	mkdir("log/seewhatLogs",0777);
	parentClintpid=getpid();
	struct sigaction sinyal;

	sinyal.sa_handler = killhander;
	sigemptyset(&sinyal.sa_mask);
	sigaddset(&sinyal.sa_mask, SIGINT);
	if (sigaction(SIGINT, &sinyal, NULL) == -1) {
		fprintf(stderr, "Cannot set sigaction...\n");
		exit(1);
	}
	while(1){
		
		/*Create own fifo to take matrix from server*/
		
		if(flag==0)	{
			fprintf(stderr, "Running...\n");
			strcpy(myfifoname,"");
			sprintf(myfifoname,"%d",getpid());
			strcat(myfifoname,".fifo");
			mkfifo(myfifoname,0666);
			/*Write to server own pid*/
			pid_t tempid=getpid();

			write(writeServerDes,&tempid,sizeof(pid_t));
		/*Open own fifo as reader*/
			readFifo=open(myfifoname,O_RDONLY);
			
			flag=1;
		}
			/*read the matrix size*/
	
		
		float matrixNsize=matrixSize/2;
		int i,j;
		if(read(readFifo,&parentTimeServerpid,sizeof(pid_t)) &&read(readFifo,&matrixSize,sizeof(int))
		 &&read(readFifo,&serverpid,sizeof(pid_t))
		  &&read(readFifo,matrix,sizeof(float)*matrixSize*matrixSize)>0 ){
			float arr[MATRIXMAXSIZE][MATRIXMAXSIZE];
			int tempsize=matrixSize;
			for ( i = 0; i < matrixSize; ++i)
			{
				for ( j = 0; j < matrixSize; ++j)
				{
					arr[i][j]=matrix[i*matrixSize+j];
				}
			}
			int pipearr[2];
			pipe(pipearr);

			pid_t fork1=fork();
			if(fork1==0){
				float shiftedMatrix[MATRIXMAXSIZE][MATRIXMAXSIZE];
				clock_t begintime = clock();
				CalculateShiftedMatrix(arr,shiftedMatrix,matrixSize);
				clock_t endTime = clock();
				double diftimee=(double)(endTime-begintime)/CLOCKS_PER_SEC;
				MatrixArrayType temp;
				for ( i = 0; i < matrixSize; ++i)
				{
					for ( j = 0; j < matrixSize; ++j)
					{
						temp.arr[i][j]=shiftedMatrix[i][j];
						//fprintf(stderr, "%lf,", shiftedMatrix[i][j]);
					}
					//fprintf(stderr, "\n");
				}
				temp.type='S';
				temp.elapsedtime=diftimee;
				close(pipearr[0]);
				write(pipearr[1],&temp,sizeof(MatrixArrayType));
				close(pipearr[1]);
				exit(0);
			}

			pid_t fork2=fork();
			if(fork2==0){
				float convuliton[MATRIXMAXSIZE][MATRIXMAXSIZE];
				clock_t begintime = clock();
				ConvolutionCalculate(arr,convuliton,matrixSize,matrixSize);
				clock_t endTime = clock();
				double diftimee=(double)(endTime-begintime)/CLOCKS_PER_SEC;
				MatrixArrayType temp;
				for ( i = 0; i < matrixSize; ++i)
				{
					for ( j = 0; j < matrixSize; ++j)
					{
						temp.arr[i][j]=convuliton[i][j];
					}
				}

				temp.type='C';
				temp.elapsedtime=diftimee;
				close(pipearr[0]);
				write(pipearr[1],&temp,sizeof(MatrixArrayType));
				close(pipearr[1]);
				
				
				exit(0);
			}
			while(wait(NULL)!=-1 || errno==EINTR);
			MatrixArrayType temp;
			float shiftedMatrix[MATRIXMAXSIZE][MATRIXMAXSIZE];
			float convuliton[MATRIXMAXSIZE][MATRIXMAXSIZE];
				
			double convolutionTime;
			double shiftTime;
				
			close(pipearr[1]);
			while (read(pipearr[0],&temp,sizeof(MatrixArrayType))>0 )
			{
				if(temp.type=='C')
				{		
					for ( i = 0; i < matrixSize; ++i){
						for ( j = 0; j < matrixSize; ++j)
							{
								convuliton[i][j]=temp.arr[i][j];
						}
					}
					convolutionTime=temp.elapsedtime;
				}
				else if(temp.type=='S'){
					for ( i = 0; i < matrixSize; ++i){
						for ( j = 0; j < matrixSize; ++j){
							
							shiftedMatrix[i][j]=temp.arr[i][j];
						}
						
					}
					shiftTime=temp.elapsedtime;
				}
			}
			close(pipearr[0]);
		
			float result1=determinant(arr,matrixSize)-determinant(shiftedMatrix,matrixSize);
			float result2=determinant(arr,matrixSize)-determinant(convuliton,matrixSize);
			MatrixResultsType result;
			result.shiftElapsed=shiftTime;
			result.convElapsed=convolutionTime;
			result.result1=result1;
			result.result2=result2;
			result.cliendPid=getpid();
			result.requestNum=logcount;
			int writeShowResult=open("showresult.fifo",O_WRONLY);
			if(writeShowResult!=-1){
				write(writeShowResult,&result,sizeof(MatrixResultsType));
				close(writeShowResult);
			}
			char cliendLogname[255];
			strcpy(cliendLogname,"");
			sprintf(cliendLogname,"log/seewhatLogs/%d-%d",getpid(),logcount);
			++logcount;

			FILE *writeClientLog=fopen(cliendLogname,"w");
				/* write oorginal matrix*/
			char tempstr[255];
			char temp2[2];
			strcpy(tempstr,"");
			sprintf(tempstr,"Orginal Matrix =[\n");
			fwrite(tempstr,sizeof(char),strlen(tempstr),writeClientLog);
			for ( i = 0; i < matrixSize; ++i)
			{
				for ( j = 0; j < matrixSize; ++j)
				{
					char temp2[10];
					strcpy(temp2,"");
					sprintf(temp2,"%.2f,",arr[i][j]);	
					fwrite(temp2,sizeof(char),strlen(temp2),writeClientLog);

				}
				char temp2=';';
				fwrite(&temp2,sizeof(char),1,writeClientLog);
			}

			strcpy(temp2,"]\n");
			fwrite(temp2,sizeof(char),2,writeClientLog);
		
			strcpy(tempstr,"");
			sprintf(tempstr,"Shifted Matrix =[\n");
			fwrite(tempstr,sizeof(char),strlen(tempstr),writeClientLog);
			for ( i = 0; i < tempsize; ++i)
			{
				for ( j = 0; j < tempsize; ++j)
				{
					
					fprintf(writeClientLog, "%lf,",shiftedMatrix[i][j] );
				}
			
				fprintf(writeClientLog, ";");
			}


			strcpy(tempstr,"");
			sprintf(tempstr,"convuliton Matrix =[\n");
			fwrite(tempstr,sizeof(char),strlen(tempstr),writeClientLog);
			for ( i = 0; i < tempsize; ++i)
			{
				for ( j = 0; j < tempsize; ++j)
				{
						fprintf(writeClientLog, "%lf,",convuliton[i][j] );
				}
				char temp2=';';
			
				fwrite(&temp2,sizeof(char),1,writeClientLog);
			}

			strcpy(temp2,"]\n");
			fwrite(temp2,sizeof(char),2,writeClientLog);

			fclose(writeClientLog);
		}
		
		kill(serverpid,SIGUSR1);
	}

	return 0;
}
