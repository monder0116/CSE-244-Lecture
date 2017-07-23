#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* File içinde 1.arguman olarak girilen kelime aranır.
	Aranırken \n,\t,' ' karakterleri ignore edilir.
	Bulunduğu yerin ilk harf konumunu ekrana yazar*/
void grepfromFile(char *filename,char* searchword);
/*Karakterin ignore edilip edilmeyeceği kontrol edilir.
	Eğer ignore karakterse 1 return eder. Değilse 0 return eder.*/
int ignoreChar(char chr);
int main(int argc,char **args){
	if(argc!=3 || strcmp("./list",args[0])!=0)
	{
		printf("it need three argument in commend line\nEx; list findWord filename\n");
		return 1;
	}
	FILE *file=fopen(args[2],"r");
	if(file==NULL)
	{
		printf("File cannot opened!\n ");
		return 1;
	}	
	fclose(file);
	grepfromFile(args[2],args[1]);
return 0;

}
void grepfromFile(char *filename,char* findWord){
	int totalSize=0,currentSize=0;
	FILE *file;

	/* Dosyanın toplam size'ı hesaplanır */
	char takenChar;
	char *chararr;
	file=fopen(filename,"r");
	while((takenChar=fgetc(file))&&!feof(file)){
		++totalSize;
	}
	fclose(file);

	file=fopen(filename,"r");
	/*Toplam size kadar yer alınır ve ve bu alınan yerin içine dosya yazılır.*/
	chararr=(char *)malloc(sizeof(char)*totalSize);
	while((takenChar=fgetc(file)) && !feof(file)){
		chararr[currentSize]=takenChar;
		++currentSize;
	}
	/* chararr içinde aranacak kelime \t,\n,space gibi karakterler ignore 
		edilerek ara yapılır. Bulunursa row ve col numaraları ekrana yazdırılır.*/
	int i,totalFoundCount=0,rownumber=1,lastEnterindex=0,colnumber=0;
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
							totalFoundCount++;
							printf("[%d,%d] konumunda ilk karakter bulundu.\n",rownumber,colnumber );
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
	printf("Total count=%d\n",totalFoundCount);
	fclose(file);


}

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