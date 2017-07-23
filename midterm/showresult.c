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
#include "StructTypes.h"
#include <sys/stat.h>
static int readshowresult;
static FILE *writelog;
static int mainServerpid=0;
void killhander(int signo){
	if(readshowresult!=-1)
		close(readshowresult);
	char tempstr[255];
	strcpy(tempstr,"");
	if(writelog!=NULL){
		sprintf(tempstr,"Kill signal was taken");
		fwrite(tempstr,sizeof(char),strlen(tempstr),writelog);
		fclose(writelog);
	}
	kill(mainServerpid,SIGINT);
	unlink("serverpid.temp");
	unlink("showresultpid.temp");
	unlink("showresult.fifo");
	exit(0);
}	
int main(int argnum, char** args){
	if(argnum!=1){
		fprintf(stderr, "Usage=./showResult");
		return 0;
	}
	if(strcmp(args[0],"./showResult")!=0){
		fprintf(stderr, "Usage=./showResult ");
		return 0;
	}
		
	FILE *readpid=fopen("serverpid.temp","r");
	if(readpid==NULL)
	{
		fprintf(stderr, "Firstly timeServer must opened\n");
		return 0;
	}
	fclose(readpid);
	if(mkfifo("showresult.fifo",0666)==-1);
	readshowresult=open("showresult.fifo",O_CREAT|O_RDONLY);
	struct sigaction sinyal;
	sinyal.sa_handler = killhander;
	sigemptyset(&sinyal.sa_mask);
	sigaddset(&sinyal.sa_mask, SIGINT);
	if (sigaction(SIGINT, &sinyal, NULL) == -1) {
		fprintf(stderr, "Cannot set sigaction...\n");
		exit(1);
	}
	sigprocmask(SIG_BLOCK, &sinyal.sa_mask, NULL);
	
	FILE *writePid=fopen("showresultpid.temp","w");
	fprintf(writePid, "%d\n",getpid() );
	fclose(writePid);
	sigprocmask(SIG_UNBLOCK, &sinyal.sa_mask, NULL);
	MatrixResultsType result;
	int flag=0;
	while(1){

		while(read(readshowresult,&result,sizeof(MatrixResultsType))>0){
			
			sigprocmask(SIG_BLOCK, &sinyal.sa_mask, NULL);
			if(flag==0){
				readpid=fopen("serverpid.temp","r");
				fscanf(readpid, "%d", &mainServerpid);
				fclose(readpid);
				flag=1;
			}
			writelog=fopen("log/showresult.log","a+");
			fprintf(stderr, "pid=%d-%d\t, Result1=%lf\t, Result2=%lf\n",result.cliendPid,result.requestNum,		
				result.result1,		result.result2);
			char tempstr[255];
			sprintf(tempstr,"cliend Pid=%d-%d\nResult1 Time=%lf\nResult2 Time=%lf\n",result.cliendPid,result.requestNum,result.shiftElapsed,result.convElapsed);
			fwrite(tempstr,sizeof(char),strlen(tempstr),writelog);
			fclose(writelog);
			writelog=NULL;
			sigprocmask(SIG_UNBLOCK, &sinyal.sa_mask, NULL);
		}	
	}
	
	return 0;
}

