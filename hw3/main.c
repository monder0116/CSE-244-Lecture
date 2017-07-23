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
#include <fcntl.h>

/* Klasor içindeki tüm dosyalara fork oluşturarak ,istenilen kelimeyi arar
 aramak için grepfromFile fonksiyonunu kullanır.Bulduğu konum ve dosya ismini
 pipe içerisine yazar, pipe ise klasor içindeki fifo ya bu bilgileri yazar
 işlemler bitince fifo dosyası bir st klasöre aktarılıp silinir*/	
 void searchFileinDic(char *dicname,char *searchWord,int fifodis);
/* String içinde space arar index döndürür*/
 int findCharindex(char *str,char findchar);
/*Log dosyası sonuna toplam bulunan sayıyı yazar*/
 void printCountToEnd(const char *logname);
/* fork işlemi yapılıp ;child fifo oluşturur sonra searchFileinDic çalışır. parent ise 
	son fifo dosyasını okuyop log dosyasına aktarır*/
 void runAndWrite(char *dic,char *word);
 const char fifoName[]="fifo";
 const char logFileName[]="log.log";
 int main(int argc,char** argv){

 	if(argc!=3)
 	{
 		printf("%s\n","Arguman sayısı eksik!\n" );
 		printf("Kullanım ;./exe <Aranacak Kelime> <Aranacak Klasor>\n" );
 		return 1;
 	}
 	if(strcmp(argv[0],"./exe")!=0)
 	{
 		printf("Kullanım ;./exe <Aranacak Kelime> <Aranacak Klasor>\n" );
 		return 1;
 	}
 	runAndWrite(argv[2],argv[1]);

 	return 0;
 }
 void runAndWrite(char *dic,char *word){
 	char path[1024];
 	char logpath[1024];
 	int fiforeadwrite;
 	char temparr[255];
	/* Main klasorde ilk fifo oluşur*/
 	sprintf(path,"%s/%s",dic,fifoName);
 	sprintf(logpath,"%s/%s",dic,logFileName);
 	mkfifo(path,0666);
 	pid_t forkid;
 	if(forkid=fork()==0){
 		fiforeadwrite=open(path,O_CREAT|O_WRONLY);
 		searchFileinDic(dic,word,fiforeadwrite);
 		close(fiforeadwrite);
 		exit(0);
 	}
	/* Son oluşan fifo dosyasını okuyup
		 Log dosyasına yazılması*/

 	int fiforead=open(path,O_RDONLY);
 	FILE* file=fopen(logFileName,"w");
 	while(read(fiforead,temparr,255)>0){
 		if(temparr[strlen(temparr)-1]=='\n'){
 			temparr[strlen(temparr)-1]='\0';
 		}
 		char fullstr[255];
 		strcpy(fullstr,"");
 		int spaceindex=findCharindex(temparr,'\t');
 		strncat(fullstr,temparr,spaceindex+1);
 		strcat(fullstr,"Dosyasında");
 		strcat(fullstr,&temparr[spaceindex]);
 		strcat(fullstr,"'de bulunmuştur.");
 		fprintf(file, "%s\n", fullstr);
 	}
 	fclose(file);
 	close(fiforead);
 	wait(NULL);
 	unlink(path);
 	printCountToEnd(logFileName);

 }
 void printCountToEnd(const char *logname){
 	FILE *readFile=fopen(logname,"r");
 	char temp;
 	int total=0;
 	while(fscanf(readFile,"%c",&temp) && !feof(readFile))
 	{
 		if(temp=='\n')
 			total++;
 	}

 	fclose(readFile);
 	FILE *writeFile=fopen(logname,"a+");
 	fprintf(writeFile, "Total Found =%d\n",total );
 	fclose(writeFile);


 }
 int findCharindex(char *str,char findchar){
 	int i ;
 	for (i=0; i<strlen(str); ++i)
 	{
 		if(str[i]==findchar){
 			return i;
 		}
 	}
 	return 0;

 }
 void searchFileinDic(char *dicname,char*searchWord,int fifodis)
 {
 	DIR* directoryLink;
 	directoryLink=opendir(dicname);
 	if(directoryLink==NULL)
 	{
 		printf("%s Klasor açılmadı!\n",dicname);
 		return ;
 	}
 	pid_t chilid;
 	struct dirent *readFiles;
 	struct stat status;
 	int pipearr[2];
 	if(pipe(pipearr)<0)
 		perror("pipe oluşmadı");
 	char logpath[1024];
 	//fprintf(stderr, "%s\n", dicname);
 	strcpy(logpath,"");
 	sprintf(logpath,"%s/%s",dicname,fifoName);
	#if DEBUG
 	fprintf(stderr, "%s\n", logpath);
	#endif

 	while((readFiles=readdir(directoryLink))!=NULL)
 	{	
 		char path[1024];

 		sprintf(path,"%s/%s",dicname,readFiles->d_name);
 		char *tempfilename=readFiles->d_name +(int)strlen(readFiles->d_name )-1;
 		if((stat(path,&status))!=-1){
			/*Parent fork oluşturur*/
 			if((chilid=fork())==0)
 			{

 				if((S_ISDIR(status.st_mode))!=0) 
 				{
 					if(strcmp(readFiles->d_name,".")!=0 && strcmp(readFiles->d_name,"..")!=0){
						/*Sub klasor içinde fifo dosyası oluşturur daha sonra bu klasor 
						için revursive çağrıda bulunur*/
 						char oldpath[1024];
 						sprintf(oldpath,"%s/%s",path,fifoName);
 						mkfifo(oldpath,0666);
 						pid_t subpros;
 						if(subpros=fork()==0){
 							int fifowrite=open(oldpath,O_CREAT|O_WRONLY);
 							searchFileinDic(path,searchWord,fifowrite);
 							close(fifowrite);
 							exit(0);
 						}
						/*Sub klasor içindeki fifoyu üst fifoya aktarır*/
 						char str[255];
 						int fiforead=open(oldpath,O_RDONLY);
 						while(read(fiforead,str,255)>0){
 							#if DEBUG
 							fprintf(stderr, "%s den aktarıldı %s\n",readFiles->d_name,str);
 							#endif

 							write(fifodis,str,255);
 						}
 						close(fiforead);
 						wait(NULL);
 						unlink(oldpath);
 						
 					}	
 				}
 				else if(strcmp(tempfilename,"~")!=0 && strcmp(readFiles->d_name,fifoName)!=0 ) 
 				{
 					if(strcmp(readFiles->d_name,".")!=0 && strcmp(readFiles->d_name,"..")!=0 && strcmp(readFiles->d_name,logFileName)!=0){
						#if DEBUG
 						fprintf(stderr,"----------%s dosyası-----------\n", readFiles->d_name);
						#endif
			
 						grepfromFile(path,searchWord,pipearr,readFiles->d_name);
 					}
 				}
 				exit(0);

 			}
 			else if(chilid==-1)
 				perror("Fork can not created!");
 		}
 		else{
 			perror("File is not valid");
 		}
 	}
 	char temparr[255];
	/*pipe a yazılana parent okur fifoya yazar*/
 	close(pipearr[1]);

 	while(wait(NULL)!=-1 || errno==EINTR){
 		while(read(pipearr[0],temparr,255)>0)
 		{
 			write(fifodis,temparr,255);
 		}
 	
 	}
 	close(pipearr[1]);
 	close(pipearr[0]);
 	closedir(directoryLink);
 }
