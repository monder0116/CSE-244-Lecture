---Compile etmek için;
*make

---timerServer çalıştırmak için 3 parametre gereklidir;
*./timeServer t n mainfifoname
*buradaki t değeri milisaniye cinsinden time
*n değeri üretilecek 2n lik matrisin boyutu
*mainfifo ise program için oluşturulacak fifo ismi.

---seeWhat çalıştırmak için 1 parametre gereklidir.
* ./seeWhat mainfifoname


---showResult çalıştırmak parametresiz şekilde çalıştırın
* ./showResult

önemli uyarı:
N=6 dan sonra çalışma yavaşlar çünkü determinat fonksiyonu recursive şekilde çalışıyor
Determinant fonksiyonu tekrar yazılırsa iterative şekilde çalışmaya devam eder


