#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <pthread.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include "matrixProg.h"
#include "qrDecom.h"
#include "svd.h"

#define MaxClientNum 500
#define MaxMatrixSize 50
#define SharedMemTotalNum 10

int listenfd = 0;
int portNum,rowNum,colNum;
pthread_t tidArr[MaxClientNum];
pthread_attr_t threadProp[MaxClientNum];
int tidArrSize=0;
int signalFlag=0;
int ConnectingCount=1;

pid_t mainPid;
pthread_t mainThreadid;
int workPoolsize=0;

/*shared memory global değişkenler*/
int shmids[SharedMemTotalNum];
RequestMatrixType* requestSharedMatrix;
//pthread_mutex_t *mutex;

void SVDTfunc();
void QRfunc();
void inverseThreadfunc();
void *subThreadFunc(void * parm);
void createSharedMemory();
void freeSharedMemory();
void closeFreeSemefore();
void createSemafore();
void *poolThreadFunc(void * parm);
void createServerPool();
void threadSameWorks(int fileDis);

void* threadFunc(void * parm);  
void createServer();
void createServerPool();
void connectSharedMemory();
void createSubProcess();
sem_t *workpoolSem,*countertopSem,*counterbuttomSem;
void signalHander(int signo){

	if(mainPid==getpid() && mainThreadid==pthread_self()){
		fprintf(stderr, "Sinyal Alındı program kapanıyor...\n");
		int i;
	
		unlink("thread.fifo");
		closeFreeSemefore();
		
	}
	else {
		kill(0,SIGINT);
		pthread_exit(0);
	}
}


int main(int argc, char *argv[])
{
	fprintf(stderr, "Program çalışıyor\n");
	if(argc!=2 && argc!=3){
		fprintf(stderr, "Usage= ./server colnumber rownumber portnumber poolsize\n");
		return 0;
	}
	mainPid=getpid();
	portNum=atoi(argv[1]);
	if(argc==3 ){

		workPoolsize=atoi(argv[2]);
		if(workPoolsize<0){
			fprintf(stderr, "Usage= ./server colnumber rownumber  portnumber\n");
			return 0;
		}
	}

	mainThreadid=pthread_self();

	struct sigaction sinyal;
	sinyal.sa_handler = signalHander;
	sigemptyset(&sinyal.sa_mask);
	sigaddset(&sinyal.sa_mask, SIGINT);
	if (sigaction(SIGINT, &sinyal, NULL) == -1) {
		fprintf(stderr, "Cannot set sigaction...\n");
		exit(0);
	}
	
	if(argc!=3 || workPoolsize==0)
		createServer();  
	else
		createServerPool();  

	
	return 0;
}

void closeFreeSemefore(){
	sem_close(workpoolSem);
	sem_unlink("workpool_sem");
	sem_close(countertopSem);
	sem_unlink("countertop_sem");
	sem_close(counterbuttomSem);
	sem_unlink("counterbuttom_sem");
}

void createSemafore(){
	closeFreeSemefore();
	workpoolSem=sem_open("workpool_sem", O_CREAT|O_RDWR, 0644, 1);
	countertopSem=sem_open("countertop_sem", O_CREAT|O_RDWR, 0644, 1);
	counterbuttomSem=sem_open("counterbuttom_sem", O_CREAT|O_RDWR, 0644, 1);
}
void *poolThreadFunc(void * parm){

	struct sigaction sinyal;
	sinyal.sa_handler = signalHander;
	sigemptyset(&sinyal.sa_mask);
	sigaddset(&sinyal.sa_mask, SIGINT);
	if (sigaction(SIGINT, &sinyal, NULL) == -1) {
		fprintf(stderr, "Cannot set sigaction...\n");
		exit(0);
	}

	int readFifo=open("thread.fifo",O_CREAT| O_RDONLY);
	fprintf(stderr, "%ld thread Created\n",pthread_self());
	while(1){
		int filedis;
		
		if(read(readFifo,&filedis,sizeof(int))>0){
			fprintf(stderr, " %ld thread isteği aldı\n",(long)pthread_self());
			threadSameWorks(filedis);
			usleep(100);
			close(filedis);
		}

	}
	pthread_exit(0);

}
void createServerPool(){


	int connectDis = 0;
	fprintf(stderr, "Server Workerpool created and running with %d thread\n",workPoolsize);
	struct sockaddr_in serverAdres; 
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd<0){
		perror("Listenfd is unable!!");
		return ;
	}
	memset(&serverAdres, '0', sizeof(serverAdres));

	int set = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		&set, sizeof(int))) {
		perror("set socket");
	exit(0);
}
serverAdres.sin_family = AF_INET;
serverAdres.sin_addr.s_addr = htonl(INADDR_ANY);
serverAdres.sin_port = htons(portNum);
if (bind(listenfd, (struct sockaddr*)&serverAdres, sizeof(serverAdres))<0){
	perror("Bind is unsucces!");
	return ; 
}
listen(listenfd, 5); 
unlink("thread.fifo");
createSemafore();
mkfifo("thread.fifo",0666);

int i;

for ( i = 0; i < workPoolsize; ++i)
{

	pthread_attr_init(&threadProp[tidArrSize]);
	pthread_attr_setstacksize(&threadProp[tidArrSize], 10000000);
	pthread_create(&tidArr[tidArrSize], &threadProp[tidArrSize],(void *) poolThreadFunc,NULL);
	tidArrSize++;
}
int writeFifo=open("thread.fifo",  O_WRONLY);
while(1)
{
	if((connectDis = accept(listenfd, (struct sockaddr*)NULL, NULL))<0)
	{
		return;
	}
	write(writeFifo,&connectDis,sizeof(int));
}

exit(0);
}
void createServer(){

	int connectDis = 0;
	int clientFileDisArr[MaxClientNum];
	
	struct sockaddr_in serverAdres; 
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd<0){
		perror("Listenfd is unable!!");
		return ;
	}
	memset(&serverAdres, '0', sizeof(serverAdres));
	int set = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		&set, sizeof(int))) {
		perror("set socket");
	exit(0);
}
serverAdres.sin_family = AF_INET;
serverAdres.sin_addr.s_addr = htonl(INADDR_ANY);
serverAdres.sin_port = htons(portNum);
if (bind(listenfd, (struct sockaddr*)&serverAdres, sizeof(serverAdres))<0){
	perror("Bind is unsucces!");
	return ; 
}
createSemafore();
listen(listenfd, 5); 
while(1)
{
	if((connectDis = accept(listenfd, (struct sockaddr*)NULL, NULL))<0)
		break; 
	clientFileDisArr[tidArrSize]=connectDis;
	pthread_attr_init(&threadProp[tidArrSize]);
	pthread_attr_setstacksize(&threadProp[tidArrSize], 10000000);
	pthread_create(&tidArr[tidArrSize], &threadProp[tidArrSize],(void *) threadFunc,&clientFileDisArr[tidArrSize]);
	fprintf(stderr, "Connecting client number=%d\n",ConnectingCount);
	tidArrSize++;
	if(tidArrSize>50){
		int i;
		for ( i = 0; i < tidArrSize; ++i)
		{   
			pthread_join(tidArr[i],NULL);
			pthread_attr_destroy(&threadProp[i]);
		}
		tidArrSize=0;
	}

}
}
typedef struct{
	int id;
	pthread_mutex_t *mutexpointer;
}SubThreadType;
void connectSharedMemory(long id){
	if(id<0)
		id*=-1;
	shmids[0] = shmget(id,0, 0);
	requestSharedMatrix = (RequestMatrixType *) shmat(shmids[0], NULL, 0);
}

void createSubProcess(){
	pid_t pid1,pid2,pid3;
	long threadMaintid=pthread_self();
	if(threadMaintid<0)
		threadMaintid*=-1;
	if((pid1=fork())<0)
	{
		perror("fork oluşmadı");
		exit(0);
	}
	else if(pid1==0)
	{
		connectSharedMemory(threadMaintid);
		createMatrix(requestSharedMatrix->Aarray,requestSharedMatrix->Barray,	requestSharedMatrix->rowNum,requestSharedMatrix->colNum);

		requestSharedMatrix->blockSolve=0;
		close(portNum);
		exit(0);
	}
	if((pid2=fork())<0)
	{

		perror("fork oluşmadı");
		exit(0);
	}
	else if(pid2==0)
	{
		connectSharedMemory(threadMaintid);
		while(requestSharedMatrix->blockSolve==1);



		int i;
		pthread_t tid[3];
		SubThreadType parmids[3];
		pthread_mutex_t mutex;
		if (pthread_mutex_init(&mutex, NULL) != 0)
		{
			printf("\n mutex fail\n");
			exit(0);
		}
		for ( i = 0; i < 3; ++i)
		{

			parmids[i].id=i+1;
			parmids[i].mutexpointer=&mutex;
			pthread_create(&tid[i],NULL,subThreadFunc,&parmids[i]);
		}
		for ( i = 0; i < 3; ++i)
		{
			pthread_join(tid[i],NULL);

		}	



		requestSharedMatrix->blockVerify=0;


		close(portNum);
		exit(0);
	}
	if((pid3=fork())<0)
	{
		perror("fork oluşmadı");
		exit(0);
	}
	else if(pid3==0)

	{
		connectSharedMemory(threadMaintid);
		while(requestSharedMatrix->blockVerify==1);
		
		requestSharedMatrix->errorQR=calculateError(requestSharedMatrix->Aarray,
			requestSharedMatrix->QRresults,requestSharedMatrix->Barray ,requestSharedMatrix->rowNum,requestSharedMatrix->colNum);
		requestSharedMatrix->errorSVD=calculateError(requestSharedMatrix->Aarray,
			requestSharedMatrix->SVDresults,requestSharedMatrix->Barray ,requestSharedMatrix->rowNum,requestSharedMatrix->colNum);
		requestSharedMatrix->errorInverse=calculateError(requestSharedMatrix->Aarray,
			requestSharedMatrix->inverseResults,requestSharedMatrix->Barray ,requestSharedMatrix->rowNum,requestSharedMatrix->colNum);



		close(portNum);

		exit(0);
	}
}

void SVDfunc(){

	double w[MaxMatrixSize],v[MaxMatrixSize][MaxMatrixSize];
	int i,j;

	for ( i = 0; i < MaxMatrixSize; ++i)
	{
		w[i]=1;
		for ( j = 0; j < MaxMatrixSize; ++j)
		{
			v[i][j]=1;
		}
	}

	solveWithSvd(requestSharedMatrix->Aarray, w, v, requestSharedMatrix->rowNum, requestSharedMatrix->colNum, requestSharedMatrix->Barray, requestSharedMatrix->SVDresults);


}
void QRfunc(){
	int i;
	for (i = 0; i < requestSharedMatrix->colNum; ++i)
	{
		requestSharedMatrix->QRresults[i];

	}


}
void inversefunc(){
	pseudoinverseCalculator(requestSharedMatrix->Aarray,requestSharedMatrix->Barray,requestSharedMatrix->inverseResults,requestSharedMatrix->rowNum,requestSharedMatrix->colNum);



}
void *subThreadFunc(void * parm){
	
	SubThreadType *param=(SubThreadType *) parm;
	pthread_mutex_lock(param->mutexpointer);
	int id=param->id;

	if(id==1){

		SVDfunc();
		
	}
	else if(id==2){

		QRfunc();


	}
	else{


		inversefunc();

	}
	pthread_mutex_unlock(param->mutexpointer);
	pthread_exit(0);
}
void createSharedMemory(){ 
	long id=pthread_self();
	if(id<0)
		id*=-1;
	shmids[0] = shmget(id,sizeof(RequestMatrixType), IPC_CREAT|0666);
	requestSharedMatrix = (RequestMatrixType *) shmat(shmids[0], NULL, 0);
	requestSharedMatrix->blockSolve=1;
	requestSharedMatrix->blockVerify=1;
}
void freeSharedMemory(){
	shmdt(requestSharedMatrix);
	shmctl(shmids[0],IPC_RMID,NULL);
}


void threadSameWorks(int fileDis){
	sem_wait(countertopSem);
	++ConnectingCount;
	sem_post(countertopSem);
	clock_t begin=clock();
	int colNum,rowNum;

	read(fileDis,&colNum,sizeof(int));
	read(fileDis,&rowNum,sizeof(int));


	createSharedMemory();
	requestSharedMatrix->rowNum=rowNum;
	requestSharedMatrix->colNum=colNum;
	createSubProcess();
	while(wait(NULL)!=-1);
	clock_t end=clock();
	float avaragetime=(float)(end-begin)/CLOCKS_PER_SEC;
	requestSharedMatrix->averageTime=avaragetime;
	write(fileDis,requestSharedMatrix,sizeof(RequestMatrixType));
	long id=pthread_self();
	char filename[50];
	sprintf(filename,"Logs/server/%ld",id);
	FILE *file=fopen(filename,"w");
	fprintf(file, "A matrix;\n" );
	int i,j;
	for ( i = 0; i < requestSharedMatrix->rowNum; ++i)
	{
		for ( j = 0; j < requestSharedMatrix->colNum; ++j)
		{
			fprintf(file, "%lf , ",requestSharedMatrix->Aarray[i][j] );
		}
		fprintf(file, "\n");
	}
	fprintf(file, "B matrix;\n" );
	for ( i = 0; i < requestSharedMatrix->rowNum; ++i)
	{

		fprintf(file, "%lf ,\n",requestSharedMatrix->Barray[i] );
	}

	fprintf(file, "\nQR result;\n" );
	for ( i = 0; i < requestSharedMatrix->colNum; ++i)
	{
		
		fprintf(file, "%lf , ",requestSharedMatrix->QRresults[i] );
		
		fprintf(file, "\n");
	}
	fprintf(file, "SVD result;\n" );
	for ( i = 0; i < requestSharedMatrix->colNum; ++i)
	{
		
		fprintf(file, "%lf , ",requestSharedMatrix->SVDresults[i] );
		
		fprintf(file, "\n");
	}
	fprintf(file, "pseudo-inverse result;\n" );
	for ( i = 0; i < requestSharedMatrix->colNum; ++i)
	{
		
		fprintf(file, "%lf , ",requestSharedMatrix->inverseResults[i] );
		
		fprintf(file, "\n");
	}
	fprintf(file, "QR result error |e|=%lf\n",requestSharedMatrix->errorQR );
	fprintf(file, "SVD result error |e|=%lf\n",requestSharedMatrix->errorSVD );
	fprintf(file, "pseudo-inverse result error |e|=%lf\n",requestSharedMatrix->errorInverse );

	fclose(file);
		sem_wait(countertopSem);

	--ConnectingCount;
	sem_post(countertopSem);
}

void* threadFunc(void * parm){
	
	struct sigaction sinyal;
	sinyal.sa_handler = signalHander;
	sigemptyset(&sinyal.sa_mask);
	sigaddset(&sinyal.sa_mask, SIGINT);
	if (sigaction(SIGINT, &sinyal, NULL) == -1) {
		fprintf(stderr, "Cannot set sigaction...\n");
		exit(0);
	}
	int *fileDis=(int *) parm;
	threadSameWorks(*fileDis);


	close(*fileDis);
	

	pthread_exit(0);
}
