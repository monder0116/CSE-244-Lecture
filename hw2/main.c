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
#define Log_File_Name "log.log"

/* Klasor içindeki tüm dosyalara fork oluşturarak ,istenilen kelimeyi arar
 aramak için grepfromFile fonksiyonunu kullanır.*/
void searchFileinDic(char *dicname,char *searchWord,FILE *logFilePointer);
/*Log dosyası sonuna toplam bulunan sayıyı yazar*/
void printCountToEnd(char *logname);


int main(int argc,char** argv){

	if(argc!=3)
	{
		printf("%s\n","Arguman sayısı eksik!\n" );
		printf("Kullanım ;./listdir <Aranacak Kelime> <Aranacak Klasor>\n" );
		return 1;
	}
	if(strcmp(argv[0],"./listdir")!=0)
	{
		printf("Kullanım ;./listdir <Aranacak Kelime> <Aranacak Klasor>\n" );
		return 1;
	}

	FILE *mainLogfile=fopen(Log_File_Name,"w");
	if(mainLogfile==NULL)
	{
		perror("Klasor Açılmadı");
		return 1;
	}
	searchFileinDic(argv[2],argv[1],mainLogfile);
	fclose(mainLogfile);
	printCountToEnd(Log_File_Name);
	return 0;
}
void printCountToEnd(char *logname){
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
void searchFileinDic(char *dicname,char*searchWord,FILE* logFilePointer)
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
	while((readFiles=readdir(directoryLink))!=NULL)
	{	
			char path[1024];
		sprintf(path,"%s/%s",dicname,readFiles->d_name);
		char *tempfilename=readFiles->d_name +(int)strlen(readFiles->d_name )-1;
		if((stat(path,&status))!=-1){
		
			if((chilid=fork())==0)
			{

				if((S_ISDIR(status.st_mode))!=0)
				{
					if(strcmp(readFiles->d_name,".")!=0 && strcmp(readFiles->d_name,"..")!=0){
						
						searchFileinDic(path,searchWord,logFilePointer);
					}	
				}
				else if(strcmp(tempfilename,"~")!=0 && strcmp(readFiles->d_name,Log_File_Name)!=0 ) 
				{
					if(strcmp(readFiles->d_name,".")!=0 && strcmp(readFiles->d_name,"..")!=0){
						#if DEBUG
						fprintf(stderr,"----------%s dosyası-----------\n", readFiles->d_name);
						#endif
						//fprintf(logFilePointer, "%s ",readFiles->d_name );
						grepfromFile(path,	searchWord,logFilePointer,readFiles->d_name);

					}
				}
				exit(0);

			}
			else if(chilid==-1)
				perror("Fork can not created!");
			else
				while(wait(NULL)==-1 && errno==EINTR);
		}
		else{
			perror("File is not valid");
		}
	}
	closedir(directoryLink);
}
