/*hey i just met u and this is crazy so im gonna test git bye*/

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
#include <sys/wait.h>
#include <signal.h>

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_SAIR_AGORA "agora"

#define MAXARGS 3
#define BUFFER_SIZE 100
#define MAX_PROCESSES 20


int main (int argc, char** argv) {

	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE];

	int process_counter = 0; 
	int pid_vector[MAX_PROCESSES];

	inicializarContas();

	printf("Bem-vinda/o ao i-banco\n\n");
	  
	while (1) {
		int numargs;
	
		numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);


		/*WORKING */

		/* 
		 * EOF (end of file) do stdin ou comando SAIR
		 * arg1(optional) char* "agora"
		 * in case of 1 argument forces quick end to all processes
		 * else it will wait for everything to be done before exiting
		 */
		if ( numargs < 0 ||
			(numargs > 0 && 
			(strcmp(args[0], COMANDO_SAIR) == 0) )){
			
			int i;
			int status;
			int pid;

			if (numargs > 2 || numargs < 1) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIR);
				continue;
			}

			printf("O i-banco vai terminar\n");

			if (numargs == 2 && strcmp(args[1], COMANDO_SAIR_AGORA)){


				for(i = 0; i < process_counter; i++)
					kill(pid_vector[i], SIGUSR1);
			} 

			

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
	

		/* 
		 * Debitar 
		 * arg 1 int - idConta
		 * arg 2 int - valor
		 * debita ao idConta o valor  
		 */
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

		/*
		 * Creditar 
		 * arg 1 int - idConta
		 * arg 2 int - valor
		 * credita ao idConta o valor
		 */
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


		/*
		 * Ler Saldo
		 * arg 1 int - idConta
		 * le o saldo da conta idConta 
		 */
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



		/* Simular 
		 * arg 1 int - nr_de_anos
		 * faz a simulacao dos nr_de_anos sob as contas existentes
		 */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
			
			if (numargs != 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
				continue;
			}
			
			int pid = fork();




			if (pid == 0){
				simular(atoi(args[1]));
				exit(EXIT_SUCCESS);

			}

			pid_vector[process_counter] = pid;
			process_counter++;

			/* use fork to run simular in child process
			 * keep track of all the PID (vector?) so it's possible
			 * to wait for them on our next step which is exit */

		}


		else {
			printf("Comando desconhecido. Tente de novo.\n");
		}
	} 
}