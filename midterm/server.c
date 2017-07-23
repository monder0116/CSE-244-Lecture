#include "stdio.h"
#include "stdlib.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "matrixCalculator.h"
#include <sys/stat.h>
#define MAXCLIENTNUM 100
#define MATRIXMAXSIZE 25

typedef struct{
	pid_t pidnum;
	double starttime;


}ClientType;

static int flag=0;
static pid_t parentpid;
static char mainFifoName[255];
static int mainFifoDes;
static pid_t cliendArr[MAXCLIENTNUM];
static int cliendArrSize=0;
static FILE *logfile;
static int logcount=0;
typedef struct 
{
	pid_t clientpid;
	float determinant;
	float elastime;
	int logcount;

}LogInfType;
void SentMatrixToClient(pid_t clientpid,int matrixSize,int *logpipe){


	char fifoname[20];
	sprintf(fifoname,"%d.fifo",clientpid);
	int writeClientFifodes=open(fifoname,O_CREAT|O_WRONLY);
	int matrix4nsize=4*matrixSize*matrixSize;
	float *matrix=(float *)malloc(sizeof(float)*matrix4nsize);
	int i,j;
	srand(time(NULL));
	int matrix2nsize=matrixSize*2;  
	float detarr[MATRIXMAXSIZE][MATRIXMAXSIZE];
	float determinantResult=0;
	clock_t begin=clock();
	while(determinantResult==0){
		float tempNmatrix[MATRIXMAXSIZE][MATRIXMAXSIZE];
		j=0;
		for (i = 0; i < matrix4nsize; ++i){

			matrix[i]= rand()% 15 ;

			if(i%matrixSize==0){
				if(j==matrixSize)
				{
					float tempdeter=determinant(tempNmatrix,matrixSize);
					if(tempdeter==0)
						i-=matrixSize;
					j=0;
				}else{
					tempNmatrix[i%matrixSize][j]=matrix[i];
					j++;
				}
			}

		}
		for ( i = 0; i < matrix2nsize; ++i)
		{
			for ( j = 0; j < matrix2nsize; ++j)
			{
				detarr[i][j]=matrix[i*matrix2nsize+j];

			}
		}
		for ( i = matrix2nsize; i < MATRIXMAXSIZE; ++i)
		{
			for (j = matrix2nsize; j < MATRIXMAXSIZE; ++j)
			{
				detarr[i][j]=0;
			}
		}
		
		determinantResult =determinant(detarr,matrix2nsize);
	}
	clock_t end=clock();
	LogInfType templog;
	templog.determinant=determinantResult;
	float tempclock=(float)(end-begin)/CLOCKS_PER_SEC;
	templog.clientpid=getpid();
	templog.elastime=tempclock;
	templog.logcount=logcount++;
	close(logpipe[0]);
	write(logpipe[1],&templog,sizeof(LogInfType));
	pid_t serverpid=getpid();
	write(writeClientFifodes,&parentpid,sizeof(pid_t));
	write(writeClientFifodes,&matrix2nsize,sizeof(int));
	write(writeClientFifodes,&serverpid,sizeof(pid_t));
	write(writeClientFifodes,matrix,sizeof(float)*matrix2nsize*matrix2nsize);
	close(writeClientFifodes);
	free(matrix);

}

void sighand(int i){
	flag=1;
}
void killhander(int signo){
	if(signo==SIGINT && parentpid==getpid()){
		kill(0,SIGINT);
		fprintf(stderr, "CTRL+C signal was taken\n");
		int i;
		for ( i = 0; i < cliendArrSize; ++i)
		{
			kill(cliendArr[i],SIGINT);
		}
		close(mainFifoDes);
		FILE* tempread=fopen("showresultpid.temp","r");
		int tempid=0;
		if(tempread!=NULL){
			fscanf(tempread,"%d",&tempid);
			fclose(tempread);
			kill(tempid,SIGINT);
		}
		unlink("serverpid.temp");
		unlink("showresultpid.temp");
		unlink("showresult.fifo");
		unlink(mainFifoName);
		exit(0);

	}
	else if(signo==SIGINT && parentpid!=getpid()){
		fclose(logfile);
		exit(0);
	}




}

int main(int argnum,char **args){


	///timeServer <ticks in miliseconds> <n> <mainpipename>
	
	if(argnum!=4){
		fprintf(stderr, "Usage=./timeServer <ticks in miliseconds> <n> <mainpipename>");
		return 0;
	}
	if(strcmp(args[0],"./timeServer")!=0){
		fprintf(stderr, "Usage=./timeServer <ticks in miliseconds> <n> <mainpipename>");
		return 0;
	}


	const int matrixSize=atoi(args[2]);
	const float waitingtime=atof(args[1])*0.001;
	strcpy(mainFifoName,args[3]);
	mkfifo(mainFifoName,0666);
	mainFifoDes=open(mainFifoName,O_CREAT| O_RDONLY ) ;

	pid_t clientpid;
	clock_t  begintime=clock();
	parentpid=getpid();
	int temp3=getpid();
	FILE *writetemppid=fopen("serverpid.temp","w");
	fprintf(writetemppid, "%d\n",temp3 );
	fclose(writetemppid);
	struct sigaction sinyal;
	sinyal.sa_handler = killhander;
	sigemptyset(&sinyal.sa_mask);
	sigaddset(&sinyal.sa_mask, SIGINT);
	if (sigaction(SIGINT, &sinyal, NULL) == -1) {
		fprintf(stderr, "Cannot set sigaction...\n");
		exit(1);
	}
	int logpipe[2];
	if(pipe(logpipe)<0){
		perror("pipe çalışmadı");
	}

	pid_t logfork=fork();
	if(logfork==0){
		LogInfType templog;
		logpipe[1];
		logfile=fopen("log/timeserver.log","w");
		while(1){
			while(read(logpipe[0],&templog,sizeof(LogInfType))>0){
				fprintf(logfile, "clientpid=%d-%d  createdTime=%lf determinant=%lf\n",templog.clientpid,templog.logcount,templog.elastime ,templog.determinant);
			}

		}


	}
	while(1){
		
		sleep(waitingtime);
		while(read(mainFifoDes,&clientpid,sizeof(pid_t))>0){
			cliendArr[cliendArrSize++]=clientpid;
			clock_t endtime = clock();
			pid_t tempPid=fork();
			if(tempPid==0){
				close(mainFifoDes);
				struct sigaction sa;
				sa.sa_handler = sighand;
				sigemptyset(&sa.sa_mask);
				sigaddset(&sa.sa_mask, SIGUSR1);

				if (sigaction(SIGUSR1, &sa, NULL) == -1) {
					fprintf(stderr, "Cannot set sigaction...\n");
					exit(1);
				}
				do
				{
				/* signal handler olustur*/
					flag=0;
					sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
					SentMatrixToClient(clientpid,matrixSize,logpipe);
					sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);
					pause();
					
				}while(flag==1);
				exit(0);
			}

		}

	}

	return 0;





}
