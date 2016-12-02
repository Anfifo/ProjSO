CFLAGS = -pthread -Wall -pedantic -g -c 

all: i-banco i-banco-terminal


i-banco: contas.o i-banco.o 
	gcc -pthread -o i-banco contas.o i-banco.o 

i-banco-terminal: commandlinereader.o i-banco-terminal.o
	gcc -o i-banco-terminal i-banco-terminal.o commandlinereader.o 

	
commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc $(CFLAGS) commandlinereader.c

contas.o: contas.c contas.h
	gcc $(CFLAGS) contas.c

i-banco.o: contas.o i-banco.c 
	gcc $(CFLAGS) i-banco.c 

i-banco-terminal.o: commandlinereader.o i-banco-terminal.c
	gcc $(CFLAGS) i-banco-terminal.c

run:
	gnome-terminal -e ~/Documents/SO/ProjSO/exercicio4/i-banco; gnome-terminal -e ~/Documents/SO/ProjSO/exercicio4/i-banco-terminal

clean:
	rm -f *.o i-banco i-banco-terminal *.txt i-banco-pipe 1*