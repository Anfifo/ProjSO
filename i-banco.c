/*
// Projeto SO - exercicio 1, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#include "commandlinereader.h"
#include "contas.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"

#define MAXARGS 3
#define BUFFER_SIZE 100


int main (int argc, char** argv) {

	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE];

	int process_counter = 0; 

	inicializarContas();

	printf("Bem-vinda/o ao i-banco\n\n");
	  
	while (1) {
		int numargs;
	
		numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

		/* EOF (end of file) do stdin ou comando "sair" */
		if (numargs < 0 ||
			(numargs > 0 && (strcmp(args[0], COMANDO_SAIR) == 0))) {
			
			int i;
			int status;
			int pid;

			if (numargs > 2 || numargs < 1) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIR);
				continue;
			}

			printf("O i-banco sucks, %d\n", process_counter);

			for (i = 0; i < process_counter; i++){
				pid = wait (&status);
				
				if (WIFEXITED(status))
					printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n", pid);

				else 
					printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n", pid);
			}          
			
			exit(EXIT_SUCCESS);
		}
	

		else if (numargs == 0)
			/* Nenhum argumento; ignora e volta a pedir */
			continue;
	

		/* Debitar */
		else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
			int idConta, valor;
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
			   continue;
			}

			idConta = atoi(args[1]);
			valor = atoi(args[2]);

			if (debitar (idConta, valor) < 0)
			   printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, idConta, valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
		}

		/* Creditar */
		else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
			int idConta, valor;
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
				continue;
			}

			idConta = atoi(args[1]);
			valor = atoi(args[2]);

			if (creditar (idConta, valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
		}

		/* Ler Saldo */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			int idConta, saldo;

			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}
		
			idConta = atoi(args[1]);
			saldo = lerSaldo (idConta);
		
			if (saldo < 0)
				printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, idConta);
			else
				printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
		}

		



		/* WORKING ON IT */



		/* Simular */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
			
			if (numargs != 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
				continue;
			}
			
			int pid = fork();

			process_counter++;

			if (pid == 0){
				simular(atoi(args[1]));
				exit(1);

			}

			/* use fork to run simular in child process
			 * keep track of all the PID (vector?) so it's possible
			 * to wait for them on our next step which is exit */



			printf("We hope it works\n");
		}

		
		/* Sair */
		else if (strcmp(args[0], COMANDO_SAIR) == 0) {


			}

			/* implementar o "sair agora" */








		else {
			printf("Comando desconhecido. Tente de novo.\n");
		}
	} 
}