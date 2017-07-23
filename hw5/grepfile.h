#ifndef GREPFILE_H
#define GREPFILE_H
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
	/* grepfromFile fonksiyonunu tread çalıştırdığı için parametresi void*
		bu yüzden gelen parametreleri bu struct yapısına aktarma işlemi ile
		alınır*/
	typedef struct {
		char filename[255];
		char searchword[255];
		FILE *mainLogfile;

	}GrepFuncParameter;
	
	extern sem_t *dicCounters_sem, *searchedThreadCounters_sem,*grepFuncCenter_sem
	,*grepFuncButton_sem,*grepFuncTop_sem,*totalFile_sem;
	extern	int *directoriesNumber;
	extern	int *searcedFileNumber;
	extern	int *lineSearchedNumber;
	extern	int *cascadeThreadNumber;
	extern	int *searcedThreadNumber;
	extern	int *foundedNumber;
	extern	int *maxThreadNumber;
	extern	int *maxThreadTemp;
	extern	int *totalFileNumber;
	extern struct sigaction sinyal;
	extern  sig_atomic_t sinyalFlag;

	extern void signalHander(int);
	/*Dosya içinde belirtilen kelimeyi arar log dosyasına bulduğu konum bilgisini yazıp
		gerekli counterları arttıma işlemi yapar*/
	void *grepfromFile(void *);
	/*Karakterin ignore edilip edilmeyeceği kontrol edilir.
	Eğer ignore karakterse 1 return eder. Değilse 0 return eder.*/
	int ignoreChar(char chr);

#endif