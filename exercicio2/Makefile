CFLAGS = -pthread -Wall -pedantic -g -c 

i-banco: commandlinereader.o contas.o i-banco.o inputProcessing.o
	gcc -pthread -o i-banco commandlinereader.o contas.o i-banco.o inputProcessing.o

inputProcessing.o: inputProcessing.c inputProcessing.h
	gcc $(CFLAGS) inputProcessing.c

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc $(CFLAGS) commandlinereader.c

contas.o: contas.c contas.h
	gcc $(CFLAGS) contas.c

i-banco.o: commandlinereader.o contas.o i-banco.c
	gcc $(CFLAGS) i-banco.c

clean:
	rm -f *.o i-banco