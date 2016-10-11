CFLAGS = -Wall -g

main: commandlinereader.o contas.o i-banco.o
	gcc -o main commandlinereader.o contas.o i-banco.o

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc $(CFLAGS) -c commandlinereader.c

contas.o: contas.c contas.h
	gcc $(CFLAGS) -c contas.c

i-banco.o: commandlinereader.o contas.o i-banco.c
	gcc $(CFLAGS) -c i-banco.c

clean:
	rm -f *.o main