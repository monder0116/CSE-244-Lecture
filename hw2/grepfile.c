#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "grepfile.h"


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
int grepfromFile(char *filename,char* findWord,FILE* logFileP,char *nameOnly){
	int totalSize=0,currentSize=0;
	FILE *file;

	/* Dosyanın toplam size'ı hesaplanır */
	char takenChar;
	char *chararr;
	file=fopen(filename,"r");
	if(file==NULL){
		printf("file açılmadı\n");
		return;
	}else
	#if DEBUG
	fprintf(stderr,"%s açıldı\n",filename);
	#endif
	while((takenChar=fgetc(file))&&!feof(file)){
		++totalSize;
	}
	fclose(file);

	file=fopen(filename,"r");
	/*Toplam size kadar yer alınır ve ve bu alınan yerin içine dosya yazılır.*/

	chararr=(char *)malloc(sizeof(char)*totalSize);
	chararr[totalSize-1]='\0';

	while((takenChar=fgetc(file)) && !feof(file)){
		chararr[currentSize]=takenChar;
		++currentSize;
	}
	/* chararr içinde aranacak kelime \t,\n,space gibi karakterler ignore 
		edilerek ara yapılır. Bulunursa row ve col numaraları ekrana yazdırılır.*/
	int i,tempFoundCount=0,rownumber=1,lastEnterindex=0,colnumber=0;
	for (i = 0; i < currentSize; ++i)
	{
		++colnumber;
		if(ignoreChar(chararr[i])==0 ){

			int flag=0,j;
			int lookedword=0;
			for (j = i;j<totalSize && lookedword<strlen(findWord); ++j)
			{
				if(ignoreChar(chararr[j])==0)
				{
					if(findWord[lookedword]!=chararr[j])
					{
						flag=1;

					}else{
						if(flag==0 && lookedword==(strlen(findWord)-1)){
							tempFoundCount++;
							fprintf(logFileP,"%s dosyasında [%d,%d] konumunda ilk karakter bulundu.\n",nameOnly,rownumber,colnumber );
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
	free(chararr);
	fclose(file);
	return tempFoundCount;


}