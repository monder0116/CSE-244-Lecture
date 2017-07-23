all:
	gcc -c server.c
	gcc -c client.c
	gcc -c matrixCalculator.c
	gcc -c showresult.c
	gcc -o timeServer server.o  matrixCalculator.o -lm
	gcc -o seeWhat client.o matrixCalculator.o -lm
	gcc -o showResult showresult.o
clean:
	rm -rf *.o *.fifo log/seewhatLogs/* log/*.log showResult seeWhat timeServer
