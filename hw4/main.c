#include "stdio.h"
#include "stdlib.h"
#include "dirent.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "string.h"
#include "grepfile.h"
#include <pthread.h>
#include <stdint.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/resource.h>
#define Log_File_Name "log.txt"

/* Klasor içindeki dosyalar için tread ,klasorler için fork oluşturup herbir
	tread grepfromFile fonkisyonunu kullanarak log dosyasına yazma ve 
	counterları arttırma işlemi yapar. Önceliği ayarlamak için isimli semaphore
	kullanılmıştır.*/
void searchFileinDic(char *dicname,char *searchWord,FILE *logFilePointer);
/*Log dosyası sonuna toplam bulunan sayıyı yazar*/
void printCountToEnd();
/*Oluşturulan global değişkenlerde alınan bir yer varsa iade eder*/
void freeGlobalVariables(void);
/*Global değişkenleri initialize eder*/
void initializeGlobalVariables(void);
/*Oluşan global counterları initialize eder*/
void initializeCounters();
/*oluşan isimli semaforları siler*/
void unlinkSemFiles();
/* Tüm counterları ekrana yazar*/

void printCounters();
/*Fork işleminden sonra oluşan forkun hesapladığı counterları 
	parent counterına aktarmak için bu struct yapısı kullanıldı.*/
typedef struct 
{
	int directoriesNumber;
	int searcedFileNumber;
	int lineSearchedNumber;
	char cascadeThreadNumbers[100];
	int searcedThreadNumber;
	int foundedNumber;
	int maxThreadNumber;
	int maxThreadTemp;
	int totalFileNumber;
}CountersType;
/*-------------Gerekli global değişkenler----------------*/
int directoriesNumber;
int searcedFileNumber;
int lineSearchedNumber;
char cascadeThreadNumbers[100];
int searcedThreadNumber;
int foundedNumber;
int maxThreadNumber;
int maxThreadTemp;
int totalFileNumber;

pthread_t mainThreadid;
pid_t mainProcessid;
FILE *mainLogfile=NULL;
struct sigaction sinyal;
clock_t begintime,endtime;


sem_t *dicCounters_sem, *searchedThreadCounters_sem,*grepFuncCenter_sem
,*grepFuncButton_sem,*grepFuncTop_sem,*totalFile_sem,*cascadeThread_sem;
/*-------------------------------------------------------*/

void signalHander(int signo){


	if(mainThreadid==pthread_self() && mainProcessid==getpid())
	{
		fprintf(stderr, "\n%d Sinyali alındı program durduruluyor..\n",signo);
		endtime=clock();
		while(wait(NULL)!=-1 || errno==EINTR);
		freeGlobalVariables();
		if(mainLogfile!=NULL){
			printCountToEnd();
		}
		printCounters();
		unlinkSemFiles();
		exit(0);


	}
	pthread_exit(NULL);



}
void printCountToEnd(){
	fprintf(mainLogfile, "Toplam bulunan =%d\n",foundedNumber );
	fclose(mainLogfile);


}
void printCounters(){
	double diftimee=(double)(endtime-begintime)/CLOCKS_PER_SEC;
	fprintf(stderr, "Total number of strings found :%d\n",foundedNumber );
	fprintf(stderr, "Number of directories searched:%d\n",directoriesNumber );
	fprintf(stderr, "Number of files searched: %d\n", totalFileNumber);
	fprintf(stderr, "Number of lines searched:%d\n",lineSearchedNumber );
	fprintf(stderr, "Number of search threads created: %d\n",searcedThreadNumber );
	fprintf(stderr, "Number of cascade threads created:%s\n",cascadeThreadNumbers );
	fprintf(stderr, "Max # of threads running concurrently:%d\n",maxThreadNumber);
	fprintf(stderr, "Total run time, in milliseconds.:%lf\n",diftimee );

}
int main(int argc,char** argv){

	if(argc!=3)
	{
		printf("%s\n","Arguman sayısı eksik!\n" );
		printf("Kullanım ;./grephTh <Aranacak Kelime> <Aranacak Klasor>\n" );
		return 1;
	}
	if(strcmp(argv[0],"./grephTh")!=0)
	{
		printf("Kullanım ;./grephTh <Aranacak Kelime> <Aranacak Klasor>\n" );
		return 1;
	}

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
	initializeGlobalVariables();
	begintime=clock();
	searchFileinDic(argv[2],argv[1],mainLogfile);
	endtime=clock();
	printCountToEnd();
	printCounters();

	freeGlobalVariables();
	//printCountToEnd(Log_File_Name);
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

	int filecount=0;
	pthread_t tidArr[1000];
	int tidArrSize=0;
	GrepFuncParameter paramArr[1000];
	int paramArrSize=0;
	struct dirent *readFiles;
	struct stat status;
	CountersType temp;
	int pipeArr[2];
	pipe(pipeArr);
	while((readFiles=readdir(directoryLink))!=NULL)
	{	
		char path[1024];
		sprintf(path,"%s/%s",dicname,readFiles->d_name);
		char *tempfilename=readFiles->d_name +(int)strlen(readFiles->d_name )-1;
		if((stat(path,&status))!=-1){
			pid_t chilid;
			sem_wait(totalFile_sem);

			(totalFileNumber)++;	
			sem_post(totalFile_sem);

			if((S_ISDIR(status.st_mode))!=0)
			{

				if(strcmp(readFiles->d_name,".")!=0 && strcmp(readFiles->d_name,"..")!=0){
					sem_wait(dicCounters_sem);
					(directoriesNumber)++;
					sem_post(dicCounters_sem);
					pid_t child=fork();
					if(child==0){
						/*counterlar sıfırlanır sadece o klasor içindeki
							counter bilgisi alınması için*/
						initializeCounters();
						searchFileinDic(path,searchWord,logFilePointer);
						/*counterlar searchFileinDic içerisinde değiştiği için
						bu bilgileri pipe ile parenta gönderilir */
						temp.directoriesNumber=directoriesNumber;
						temp.searcedFileNumber=searcedFileNumber;
						temp.lineSearchedNumber=lineSearchedNumber;
						temp.searcedThreadNumber=searcedThreadNumber;
						temp.foundedNumber=foundedNumber;
						temp.maxThreadTemp=maxThreadTemp;
						temp.maxThreadNumber=maxThreadNumber;
						temp.totalFileNumber=totalFileNumber;
						strcpy(temp.cascadeThreadNumbers,cascadeThreadNumbers);
						/*pipe a yazma işlemi gerçekleşir*/
						close(pipeArr[0]);
						write(pipeArr[1],&temp,sizeof(CountersType));
						close(pipeArr[0]);
						close(pipeArr[1]);
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
					/*grepfromFile malloc kullandığı için sinyal blocklanır oluştuktan sonra kaldırılır*/ 
					pthread_sigmask(SIG_BLOCK, &sinyal.sa_mask, NULL);
					int result=pthread_create(&tidArr[tidArrSize], NULL, grepfromFile, &paramArr[tidArrSize]);
					pthread_sigmask(SIG_UNBLOCK, &sinyal.sa_mask, NULL);
					
					if(result!=0)
						perror("Thread oluşmadı");
					tidArrSize++;
					filecount++;
					sem_wait(searchedThreadCounters_sem);
					++(searcedThreadNumber);
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
	sigprocmask(SIG_BLOCK, &sinyal.sa_mask, NULL);
	for (i = 0; i < tidArrSize; ++i)
	{
		
		pthread_join(tidArr[i],NULL);
	}
	sigprocmask(SIG_UNBLOCK, &sinyal.sa_mask, NULL);
	while(wait(NULL)!=-1 || errno==EINTR){
		/* subprocess öldüğünde pipe içerisine subklasore
			ait counterlar yazılmıştır bu bilgileri parent kendisine ekler*/
		close(pipeArr[1]);
		read(pipeArr[0],&temp,sizeof(CountersType));
		directoriesNumber+=temp.directoriesNumber;
		searcedFileNumber+=temp.searcedFileNumber;
		lineSearchedNumber+=temp.lineSearchedNumber;
		searcedThreadNumber+=temp.searcedThreadNumber;
		foundedNumber+=temp.foundedNumber;
		maxThreadTemp+=temp.maxThreadTemp;
		maxThreadNumber+=temp.maxThreadNumber;
		totalFileNumber+=temp.totalFileNumber;
		strcat(cascadeThreadNumbers,temp.cascadeThreadNumbers);
	}
	
	sem_wait(cascadeThread_sem);
	char tempstr[10];
	strcpy(tempstr,"");
	sprintf(tempstr,"%d,",filecount);
	strcat(cascadeThreadNumbers,tempstr);
	sem_post(cascadeThread_sem);

	close(pipeArr[0]);
	close(pipeArr[1]);
	closedir(directoryLink);
}

void freeGlobalVariables(void){


	sem_close(searchedThreadCounters_sem);
	sem_close(grepFuncCenter_sem);
	sem_close(grepFuncButton_sem);
	sem_close(grepFuncTop_sem);
	sem_close(totalFile_sem);
	sem_close(dicCounters_sem);
	sem_close(cascadeThread_sem);
	unlinkSemFiles();
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
void initializeCounters(){

	directoriesNumber=0;
	searcedFileNumber=0;
	lineSearchedNumber=0;
	searcedThreadNumber=0;
	foundedNumber=0;
	maxThreadTemp=0;
	maxThreadNumber=0;
	totalFileNumber=0;
	strcpy(cascadeThreadNumbers,"");

}
void initializeGlobalVariables(void){
	unlinkSemFiles();
	initializeCounters();

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
