#include "stdio.h"
#include "stdlib.h"
#include "dirent.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "string.h"
#include "grepfile.h"
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define Log_File_Name "log.txt"
#define SHARE_MEMORY_COUNT 9
/* Klasor içindeki dosyalar için tread ,klasorler için fork oluşturup herbir
	tread grepfromFile fonkisyonunu kullanarak log dosyasına yazma ve 
	counterları arttırma işlemi yapar. Önceliği ayarlamak için isimli semaphore
	kullanılmıştır.*/
	void searchFileinDic(char *dicname,char *searchWord,FILE *logFilePointer);
/*Log dosyası sonuna toplam bulunan sayıyı yazar*/
	void printCountToEnd();
/*Oluşturulan global değişkenlerde alınan bir yer varsa iade eder*/
	void closeSemafores(void);
/*Global değişkenleri initialize eder*/
	void initializeSemafores(void);
/*Oluşan global counterları initialize eder*/
	void initializeSharedMemory();
/*oluşan isimli semaforları siler*/
	void unlinkSemFiles();
	/*Shared memory oluşturur*/
	void createSharedMemory();
	/* Tüm counterları ekrana yazar*/
	void printCounters();
	/*Shared memoryi free eder*/
	void freeSharedMemory();
	/*shared mem oluşturur.*/
	void createMessageQueue();
	/* shared memoryleri siler*/
	void removeMessageQueue();
	/*Sinyal geldikten sonra durma bölgelerinde programı durdurur*/
	void closeProgram();
/*Fork işleminden sonra oluşan forkun hesapladığı counterları 
	parent counterına aktarmak için bu struct yapısı kullanıldı.*/
	typedef struct 
	{
		long id;
		int directoriesNumber;
		int searcedFileNumber;
		int lineSearchedNumber;
		int cascadeThreadNumbers;
		int searcedThreadNumber;
		int foundedNumber;
		int maxThreadNumber;
		int maxThreadTemp;
		int totalFileNumber;
	}MessageCountersType;
/*-------------Shared memory için global değişkenler----------------*/
	int *directoriesNumber;
	int *searcedFileNumber;
	int *lineSearchedNumber;
	int *cascadeThreadNumbers;
	int *searcedThreadNumber;
	int *foundedNumber;
	int *maxThreadNumber;
	int *maxThreadTemp;
	int *totalFileNumber;
	int shmids[10];
	/* sinyal geldiği zaman flag tutarak işlemlerden 
		çıkış yapılır*/
	sig_atomic_t sinyalFlag=0;
	int sinyalNo=0;

	/* Mesaj queue id */
	int msgQueueid;

	/* Fork ve main pid ve tid değerleri tutulur*/
	pthread_t forkThreadid;
	pid_t forkProcessid;
	pthread_t mainThreadid;
	pid_t mainProcessid;

	FILE *mainLogfile=NULL;
	/*Sinyal hander*/
	struct sigaction sinyal;
	clock_t begintime,endtime;
	/*isimli semafore değişkenleri*/
	sem_t *dicCounters_sem, *searchedThreadCounters_sem,*grepFuncCenter_sem
	,*grepFuncButton_sem,*grepFuncTop_sem,*totalFile_sem,*cascadeThread_sem,sinyal_sem;
/*-------------------------------------------------------*/

	void signalHander(int signo){
		sinyalNo=signo;
		sinyalFlag=1;
		if(mainThreadid==pthread_self() && mainProcessid==getpid())
			fprintf(stderr, "\n%d Sinyali alındı program durduruluyor..\n",sinyalNo);


	}
	void closeProgram(){
		if(forkThreadid==pthread_self() && forkProcessid==getpid())
		{


			while(wait(NULL)!=-1 || errno==EINTR);
			if(mainThreadid==pthread_self() && mainProcessid==getpid())
			{
				endtime=clock();
				printCounters();
				if(mainLogfile!=NULL){
					printCountToEnd();
				}
			}
			removeMessageQueue();
			freeSharedMemory();
			closeSemafores();
			unlinkSemFiles();
			exit(0);


		}

		pthread_exit(NULL);



	}
	void printCountToEnd(){
		fprintf(mainLogfile, "Toplam bulunan =%d\n",*foundedNumber );
		fclose(mainLogfile);


	}
	void printCounters(){
		double diftimee=(double)(endtime-begintime)/CLOCKS_PER_SEC;
		fprintf(stderr, "Total number of strings found :%d\n",*foundedNumber );
		fprintf(stderr, "Number of directories searched:%d\n",*directoriesNumber +1);
		fprintf(stderr, "Number of files searched: %d\n", *totalFileNumber);
		fprintf(stderr, "Number of lines searched:%d\n",*lineSearchedNumber );
		fprintf(stderr, "Number of search threads created: %d\n",*searcedThreadNumber );
		fprintf(stderr, "Number of cascade threads created:%d\n",*cascadeThreadNumbers );
		fprintf(stderr, "Max # of threads running concurrently:%d\n",*maxThreadNumber);
		fprintf(stderr, "Total run time, in milliseconds.:%lf\n",diftimee );
		fprintf(stderr, "Total Shared Memory Size=%d Byte\n",(*cascadeThreadNumbers +1)*sizeof(int)*SHARE_MEMORY_COUNT );

	}
	int main(int argc,char** argv){

		if(argc!=3)
		{
			printf("%s\n","Arguman sayısı eksik!\n" );
			printf("Kullanım ;./grephTh <Aranacak Kelime> <Aranacak Klasor>\n" );
			return 1;
		}
		if(strcmp(argv[0],"./grepSh")!=0)
		{
			printf("Kullanım ;./grephTh <Aranacak Kelime> <Aranacak Klasor>\n" );
			return 1;
		}

		DIR* directoryLink;
		directoryLink=opendir(argv[2]);
		if(directoryLink==NULL)
		{
			fprintf(stderr, "Klasor açılamadı!\n");
			return 0;
		}
		closedir(directoryLink);
		mainLogfile=fopen(Log_File_Name,"w");
		if(mainLogfile==NULL)
		{
			perror("Klasor Açılmadı");
			return 1;
		}
		sinyal.sa_handler = signalHander;
		sigemptyset(&sinyal.sa_mask);
		sigaddset(&sinyal.sa_mask, SIGINT);
		sigaddset(&sinyal.sa_mask,SIGTSTP );
		if (sigaction(SIGINT, &sinyal, NULL) == -1) {
			fprintf(stderr, "Cannot set sigaction...\n");
			exit(1);
		}
		if (sigaction(SIGTSTP, &sinyal, NULL) == -1) {
			fprintf(stderr, "Cannot set sigaction...\n");
			exit(1);
		}
		initializeSemafores();
		begintime=clock();
		searchFileinDic(argv[2],argv[1],mainLogfile);
		endtime=clock();
		printCountToEnd();
		printCounters();
		removeMessageQueue();
		closeSemafores();
		unlinkSemFiles();
		return 0;
	}
	void searchFileinDic(char *dicname,char*searchWord,FILE* logFilePointer)
	{
		DIR* directoryLink;
		directoryLink=opendir(dicname);
		if(directoryLink==NULL)
		{
			printf("%s Klasor açılmadı!\n",dicname);
			return ;
		}
		forkThreadid=pthread_self();
		forkProcessid=getpid();
		int filecount=0;
		pthread_t tidArr[1000];
		int tidArrSize=0;
		GrepFuncParameter paramArr[1000];
		int paramArrSize=0;
		createSharedMemory();
		initializeSharedMemory();
		struct dirent *readFiles;
		struct stat status;
		MessageCountersType temp;
		sem_init(&sinyal_sem, 0, 1);
		while((readFiles=readdir(directoryLink))!=NULL && sinyalFlag==0)
		{	
			char path[1024];
			sprintf(path,"%s/%s",dicname,readFiles->d_name);
			char *tempfilename=readFiles->d_name +(int)strlen(readFiles->d_name )-1;
			if((stat(path,&status))!=-1){
				pid_t chilid;
				sem_wait(totalFile_sem);

				(*totalFileNumber)++;	
				sem_post(totalFile_sem);
				if((S_ISDIR(status.st_mode))!=0)
				{

					if(strcmp(readFiles->d_name,".")!=0 && strcmp(readFiles->d_name,"..")!=0){

						sem_wait(dicCounters_sem);
						(*directoriesNumber)++;
						sem_post(dicCounters_sem);
						sem_wait(cascadeThread_sem);
						(*cascadeThreadNumbers)+=1;
						sem_post(cascadeThread_sem);
						pid_t child=fork();
						if(child==0){
						/*counterlar sıfırlanır sadece o klasor içindeki
							counter bilgisi alınması için*/
							freeSharedMemory();
							searchFileinDic(path,searchWord,logFilePointer);
							temp.directoriesNumber=*directoriesNumber;
							temp.searcedFileNumber=*searcedFileNumber;
							temp.lineSearchedNumber=*lineSearchedNumber;
							temp.searcedThreadNumber=*searcedThreadNumber;
							temp.foundedNumber=*foundedNumber;
							temp.maxThreadTemp=*maxThreadTemp;
							temp.maxThreadNumber=*maxThreadNumber;
							temp.totalFileNumber=*totalFileNumber;
							temp.cascadeThreadNumbers=(*cascadeThreadNumbers);
						/*pipe a yazma işlemi gerçekleşir*/

							msgQueueid = msgget((key_t)12345678, IPC_CREAT | S_IRUSR | S_IWUSR);
							temp.id=1;
							msgsnd(msgQueueid, &temp, sizeof(temp), 0);
							freeSharedMemory();

							closedir(directoryLink);
							exit(0);
						}
					}	
				}
				else if(strcmp(tempfilename,"~")!=0 && strcmp(readFiles->d_name,Log_File_Name)!=0 ) 
				{
					if(strcmp(readFiles->d_name,".")!=0 && strcmp(readFiles->d_name,"..")!=0){
						strcpy(paramArr[tidArrSize].filename,path);
						strcpy(paramArr[tidArrSize].searchword,searchWord);
						paramArr[tidArrSize].mainLogfile=logFilePointer;
			
						int result=pthread_create(&tidArr[tidArrSize], NULL, grepfromFile, &paramArr[tidArrSize]);
						
						if(result!=0)
							perror("Thread oluşmadı");
						tidArrSize++;
						filecount++;
						sem_wait(searchedThreadCounters_sem);
						++(*searcedThreadNumber);
						sem_post(searchedThreadCounters_sem);
					}
				}


			}
			else{
				perror("File is not valid");
			}
		} 
		int i;
	/*Sinyal esnasında oluşan treadler join edilir sonra sinyal
	görevini yapar*/
		
		for (i = 0; i < tidArrSize; ++i)
		{
			pthread_join(tidArr[i],NULL);
		}
		if(sinyalFlag==1){
			closedir(directoryLink);
			closeProgram();
		}
		while(wait(NULL)!=-1 || errno==EINTR){
		/* subprocess öldüğünde pipe içerisine subklasore
			ait counterlar yazılmıştır bu bilgileri parent kendisine ekler*/

			msgQueueid = msgget((key_t)12345678,0);
			msgrcv(msgQueueid, &temp, sizeof(temp),0, 0);
			*directoriesNumber+=temp.directoriesNumber;
			*searcedFileNumber+=temp.searcedFileNumber;
			*lineSearchedNumber+=temp.lineSearchedNumber;
			*searcedThreadNumber+=temp.searcedThreadNumber;
			*foundedNumber+=temp.foundedNumber;
			*maxThreadTemp+=temp.maxThreadTemp;
			*maxThreadNumber+=temp.maxThreadNumber;
			*totalFileNumber+=temp.totalFileNumber;
			(*cascadeThreadNumbers)+=temp.cascadeThreadNumbers;

		}
	
		closeSemafores();
		closedir(directoryLink);
	}

	void closeSemafores(void){


		sem_close(searchedThreadCounters_sem);
		sem_close(grepFuncCenter_sem);
		sem_close(grepFuncButton_sem);
		sem_close(grepFuncTop_sem);
		sem_close(totalFile_sem);
		sem_close(dicCounters_sem);
		sem_close(cascadeThread_sem);
		
	}
	void unlinkSemFiles(){
		sem_unlink("searchedThreadCounters_sem");
		sem_unlink("grepFuncCenter_sem");
		sem_unlink	("grepFuncButton_sem");
		sem_unlink	("grepFuncTop_sem");
		sem_unlink	("totalFile_sem");
		sem_unlink("diccounters_sem");
		sem_unlink("cascadeThread_sem");
	}
	void createMessageQueue(){
		key_t key;

		
		msgQueueid = msgget((key_t)12345678, IPC_CREAT | S_IRUSR | S_IWUSR);
		if (msgQueueid == -1) {
			perror("msgget");
			exit(1);
		}
	}
	void removeMessageQueue(){
		msgctl(msgQueueid,IPC_RMID, NULL);

	}
	void freeSharedMemory(){

		shmdt(directoriesNumber);
		shmdt(searcedFileNumber);
		shmdt(lineSearchedNumber);
		shmdt(searcedThreadNumber);
		shmdt(foundedNumber);
		shmdt(maxThreadTemp);
		shmdt(maxThreadNumber);
		shmdt(totalFileNumber);
		shmdt(cascadeThreadNumbers);
		int i;
		for ( i = 0; i < 9; ++i)
		{
			shmctl(shmids[i],IPC_RMID,NULL);
		}
		
	}
	void initializeSharedMemory(){
		*directoriesNumber=0;
		*searcedFileNumber=0;
		*lineSearchedNumber=0;
		*searcedThreadNumber=0;
		*foundedNumber=0;
		*maxThreadTemp=0;
		*maxThreadNumber=0;
		*totalFileNumber=0;

		*cascadeThreadNumbers=0;

	}

	void createSharedMemory(){


		
		shmids[0] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
		directoriesNumber = (int *) shmat(shmids[0], NULL, 0);
		//fprintf(stderr, "directoriesNumber=%p\n", directoriesNumber);

		shmids[1] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
		searcedFileNumber = (int *) shmat(shmids[1] , NULL, 0);



//fprintf(stderr, "searcedFileNumber=%p\n", searcedFileNumber);

		shmids[2] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
//fprintf(stderr, "lineSearchedNumber=%p\n", lineSearchedNumber);
		lineSearchedNumber = (int *) shmat(shmids[2], NULL, 0);
		shmids[3] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
		cascadeThreadNumbers = (int *) shmat(shmids[3] , NULL, 0);
//fprintf(stderr, "cascadeThreadNumbers=%p\n", cascadeThreadNumbers);

		shmids[4] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
		searcedThreadNumber = (int *) shmat(shmids[4], NULL, 0);
//fprintf(stderr, "searchedThreadCounters_semNumber=%p\n", searcedThreadNumber);

		shmids[5] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
		foundedNumber = (int *) shmat(shmids[5] , NULL, 0);
//fprintf(stderr, "foundedNumber=%p\n", foundedNumber);

		shmids[6] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
		maxThreadNumber = (int *) shmat(shmids[6] , NULL, 0);
//fprintf(stderr, "maxThreadNumber=%p\n", maxThreadNumber);

		shmids[7] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
		maxThreadTemp = (int *) shmat(shmids[7], NULL, 0);
//fprintf(stderr, "maxThreadTemp=%p\n", maxThreadTemp);

		shmids[8] = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0666);
		totalFileNumber = (int *) shmat(shmids[8], NULL, 0);
//fprintf(stderr, "totalFileNumber=%p\n", totalFileNumber);


//fprintf(stderr, "totalFileNumber=%p\n", totalFileNumber);
		


	}
	void initializeSemafores(void){
		unlinkSemFiles();

		mainThreadid=pthread_self();
		mainProcessid=getpid();

		dicCounters_sem=sem_open("diccounters_sem", O_CREAT|O_RDWR, 0644, 1);
	//fprintf(stderr, "dicCounters_sem=%p\n", dicCounters_sem);
		searchedThreadCounters_sem=sem_open("searchedThreadCounters_sem", O_CREAT|O_RDWR, 0644, 1);
	//fprintf(stderr, "searchedThreadCounters_sem=%p\n", searchedThreadCounters_sem);
		grepFuncCenter_sem=sem_open("grepFuncCenter_sem", O_CREAT|O_RDWR, 0644, 1);
	//fprintf(stderr, "grepFuncCenter_sem=%p\n", grepFuncCenter_sem);
		grepFuncButton_sem=sem_open("grepFuncButton_sem", O_CREAT|O_RDWR, 0644, 1);
	//fprintf(stderr, "grepFuncButton_sem=%p\n", grepFuncButton_sem);
		grepFuncTop_sem=sem_open("grepFuncTop_sem", O_CREAT|O_RDWR, 0644, 1);
	//fprintf(stderr, "grepFuncTop_sem=%p\n", grepFuncButton_sem);
		totalFile_sem=sem_open("totalFile_sem", O_CREAT|O_RDWR, 0644, 1);
	//fprintf(stderr, "totalFile_sem=%p\n", totalFile_sem);
		cascadeThread_sem=sem_open("cascadeThread_sem", O_CREAT|O_RDWR, 0644, 1);
	}
