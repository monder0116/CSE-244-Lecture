#include <stdio.h>	//printf
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include "matrixProg.h"


#define MaxClientNum 150
int signalFlag=0;
/* execution parametreli global değişkenlere aktarmak için*/
int rowNum,colNum;
sem_t *clientRequestSem;
pid_t mainpid;
pthread_t mainthreadid;
pthread_t tidArr[MaxClientNum];
int tidArrSize=0;
int portnum;

void closeFreeSemefore();
void createSemafore();
void createClient(int);
void *connectSocket(void*);

void signalHander(int signo){
	
	if(mainpid==getpid() && mainthreadid==pthread_self()){
		fprintf(stderr, "Sinyal Alındı program kapanıyor...\n");
		int i;
		kill(0 ,SIGINT);
		closeFreeSemefore();
		close(portnum);
		exit(0);

	}
	else
		pthread_exit(0);




}


int main(int argc , char *argv[])
{
	mainpid=getpid();
	mainthreadid=pthread_self();

	portnum=atoi(argv[4]);
	int totalThreadNum=atoi(argv[3]);

	struct sigaction sinyal;

	rowNum=atoi(argv[2]);
	colNum=atoi(argv[1]);
	mkdir("Logs/clients",0777);
	if(totalThreadNum<1)
	{
		perror("thread num invalid");
		return 0;
	}
	sinyal.sa_handler = signalHander;
	sigemptyset(&sinyal.sa_mask);
	sigaddset(&sinyal.sa_mask, SIGINT);
	if (sigaction(SIGINT, &sinyal, NULL) == -1) {
		fprintf(stderr, "Cannot set sigaction...\n");
		exit(0);
	}
	
	createClient(totalThreadNum);


	return 0;
}

void createClient(int totalThreadNum){


	
	
	int i,j;
	createSemafore();

	for ( i = 0; i < totalThreadNum && signalFlag==0; ++i)
	{

		pthread_create(&tidArr[tidArrSize], NULL,(void *) connectSocket,NULL);
	
		tidArrSize++;
	}
	for ( j = 0; j < tidArrSize; ++j)
	{
		pthread_join(tidArr[j],NULL);
		signalFlag=0;
	}

	
}
void closeFreeSemefore(){
	sem_close(clientRequestSem);
	sem_unlink("clientRequest_sem");
}
void createSemafore(){
	closeFreeSemefore();
	clientRequestSem=sem_open("clientRequest_sem", O_CREAT|O_RDWR, 0644, 1);
}

void *connectSocket(void*parm){
	int sock;
	struct sockaddr_in server;

	
	sock = socket(AF_INET , SOCK_STREAM , 0);

	if (sock == -1)
	{
		fprintf(stderr, "socket fail\n");
		kill(mainpid,SIGINT);
		pthread_exit(0);
	}
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_family = AF_INET;
	server.sin_port = htons( portnum );
	sem_wait(clientRequestSem);	
	if (  (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) )
	{	
		kill(mainpid,SIGINT);

		pthread_exit(0);
	}
	sem_post(clientRequestSem);
	write(sock,&colNum,sizeof(int));
	write(sock,&rowNum,sizeof(int));
	RequestMatrixType temp;

	read(sock,&temp,sizeof(RequestMatrixType));
	char filename[50];
	long id=pthread_self();
	if(id<0)
		id*=-1;

	sprintf(filename,"Logs/clients/%ld",id);
	FILE *file=fopen(filename,"w");
	fprintf(file, "A matrix;\n" );
	int i,j;
	for ( i = 0; i < temp.rowNum; ++i)
	{
		for ( j = 0; j < temp.colNum; ++j)
		{
			fprintf(file, "%lf , ",temp.Aarray[i][j] );
		}
		fprintf(file, "\n");
	}
	fprintf(file, "B matrix;\n" );
	for ( i = 0; i < temp.rowNum; ++i)
	{

		fprintf(file, "%lf ,\n",temp.Barray[i] );
	}

	fprintf(file, "\nQR result;\n" );
	for ( i = 0; i < temp.colNum; ++i)
	{
		
		fprintf(file, "%lf , ",temp.QRresults[i] );
		
		fprintf(file, "\n");
	}
	fprintf(file, "SVD result;\n" );
	for ( i = 0; i < temp.colNum; ++i)
	{
		
		fprintf(file, "%lf , ",temp.SVDresults[i] );
		
		fprintf(file, "\n");
	}
	fprintf(file, "pseudo-inverse result;\n" );
	for ( i = 0; i < temp.colNum; ++i)
	{
		
		fprintf(file, "%lf , ",temp.inverseResults[i] );
		
		fprintf(file, "\n");
	}
	fprintf(file, "QR result error |e|=%lf\n",temp.errorQR );
	fprintf(file, "SVD result error |e|=%lf\n",temp.errorSVD );
	fprintf(file, "pseudo-inverse result error |e|=%lf\n",temp.errorInverse );
	
	double qrs=standardDeviation(temp.QRresults,temp.rowNum);
	double svds=standardDeviation(temp.SVDresults,temp.colNum);
	double invs=standardDeviation(temp.inverseResults,temp.colNum);
	fprintf(file, "QR standard deviation=%.3lf\n",qrs );
	fprintf(file, "SVD standard deviation=%.3lf\n",svds );
	fprintf(file, "pseudo-inverse standard deviation=%.3lf\n",invs );
	fprintf(file, "Average time=%lf\n",temp.averageTime );
	fprintf(stderr,"Average time=%lf\n",temp.averageTime );
	close(portnum);
	close(sock);
	pthread_exit(0);

}
