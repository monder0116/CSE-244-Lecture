all:
	gcc -o server server.c matrixProc.c qrDecom.c svd.c vector.c  -lm -lpthread
	gcc -o client client.c matrixProc.c -lm -lpthread
clean:
	rm -f -r *.o *.fifo Logs/clients/* Logs/server/* server client
