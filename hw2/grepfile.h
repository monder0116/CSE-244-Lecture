#ifndef GREPFILE_H
#define GREPFILE_H
/* File içinde 1.arguman olarak girilen kelime aranır.
	Aranırken \n,\t,' ' karakterleri ignore edilir.
	Bulunduğu yerin ilk harf konumunu pointer adresine yazar*/
int grepfromFile(char *filename,char* searchword,FILE* mainLogfile,char *nameOnly);
/*Karakterin ignore edilip edilmeyeceği kontrol edilir.
	Eğer ignore karakterse 1 return eder. Değilse 0 return eder.*/
int ignoreChar(char chr);

#endif