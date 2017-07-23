#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "grepfile.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
int ignoreChar(char chr){
	char ignorearr[]={'\n','\t',' '};
	int arrsize=3, i;
	for(i=0;i<arrsize;i++){
		if(chr==ignorearr[i]){
			return 1;
		}
	}

	return 0;
}

void *grepfromFile(void *parms){
		
	GrepFuncParameter *inff=parms;
	char filename[255], findWord[255];
	strcpy(filename,inff->filename);
	strcpy(findWord,inff->searchword);
	FILE *logFileP=inff->mainLogfile;
	char takenChar, *chararr=NULL;
	int totalSize=0,currentSize=0;
	FILE *file;

	/* Dosyanın toplam size'ı hesaplanır */
	if(sinyalFlag==1)
		pthread_exit(NULL);
	file=fopen(filename,"r");
	if(file==NULL){
		printf("file açılmadı\n");
		return;
	}else
	while((takenChar=fgetc(file))&&!feof(file)){
		++totalSize;
	}
	fclose(file);



	file=fopen(filename,"r");
	/*Toplam size kadar ye 	r alınır ve ve bu alınan yerin içine dosya yazılır.*/

	chararr=(char *)malloc(sizeof(char)*totalSize);
	chararr[totalSize-1]='\0';

	while((takenChar=fgetc(file)) && !feof(file)){
		chararr[currentSize]=takenChar;
		++currentSize;
	}

	fclose(file);
	
	sem_wait(grepFuncTop_sem);
	(*maxThreadTemp)++;
	if(*maxThreadNumber<*maxThreadTemp)
		(*maxThreadNumber)=(*maxThreadTemp);

	sem_post(grepFuncTop_sem);
	/* chararr içinde aranacak kelime \t,\n,space gibi karakterler ignore 
		edilerek ara yapılır. Bulunursa row ve col numaraları ekrana yazdırılır.*/
	int i,tempFoundCount=0,rownumber=1,lastEnterindex=0,colnumber=0;
	for (i = 0; i < totalSize && sinyalFlag==0 ; ++i)
	{
		++colnumber;
		if(ignoreChar(chararr[i])==0 ){

			int flag=0,j;
			int lookedword=0;
			for (j = i;j<totalSize && lookedword<strlen(findWord)  && sinyalFlag==0 ; ++j)
			{
				if(ignoreChar(chararr[j])==0)
				{
					if(findWord[lookedword]!=chararr[j])
					{
						flag=1;

					}else{

						if(flag==0 && lookedword==(strlen(findWord)-1)){

							sem_wait(grepFuncCenter_sem);
							int pid=getpid();
							long unsigned tid=pthread_self();
							(*foundedNumber)++;
							fflush(logFileP);
							fprintf(logFileP,"Pid=%d - Tid=%lu : %s dosyasında [%d,%d] konumunda ilk karakter bulundu.\n",pid,tid,filename,rownumber,colnumber );
							sem_post(grepFuncCenter_sem);
							flag=1;
						}
						++lookedword;
					}
				}	
			}
		}else{
			if(chararr[i]=='\n'){
				++rownumber;
				colnumber=0;
			}
		}
	}
	rownumber--;

	sem_wait(grepFuncButton_sem);
	(*lineSearchedNumber)+=rownumber;
	(*maxThreadTemp)--;
	free(chararr);
	sem_post(grepFuncButton_sem);
	chararr=NULL;
	
	pthread_exit(NULL);

}