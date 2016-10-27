/* 
 * * * * * * * * * * * * *
 * Grupo: 4
 * Andre' Fonseca 84698
 * Isabel Dias 84726
 * 
 * added:
 * 		Comandos: "sair", "sair agora" and " simular"
 * 		funcao: apanhaSignalSIGUSR1()
 * 		Multi Processos para exercutar o Simular
 * 		Comentarios
 * * * * * * * * * * * * *
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

/* commands */
#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_SAIR_AGORA "agora"

#define OPERACAO_DEBITAR 1337
#define OPERACAO_CREDITAR 666
#define OPERACAO_LER_SALDO 007
#define OPERACAO_SIMULAR 2319


#define MAXARGS 3
#define BUFFER_SIZE 100
#define NR_MAX_PROCESSOS 20
#define NUM_TRABALHADORAS 3
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)


typedef struct {
	int operacao;
	int idConta;
	int valor;
}comando_t;



/* function that processes signal SIGUSR1 */
void apanhaSinalSIGUSR1();

/* flag changed by signal SIGUSR1 used in child process */
extern int flag;

comando_t cmd_buffer[CMD_BUFFER_DIM];

int buff_write_idx = 0;

int buff_read_idx = 0;



void initializeBuffer(){
	/* this is not how it will work :(
	int i;

	for(i = 0; i < CMD_BUFFER_DIM; i++)
		cmd_buffer[i] = NULL;
	*/
}


void addToBuffer(comando_t dualshock){
	

	/*
	* we need 
	* to use semaphores
	int i;

	while(1)
		for(i = 0; i < CMD_BUFFER_DIM; i++)
			if(cmd_buffer[i] == NULL){
				cmd_buffer[i] = dualshock;
				return;		
			}
	*/
}





int main (int argc, char** argv) {

	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE];

	int process_counter = 0; 
	int pid_vector[NR_MAX_PROCESSOS];

	inicializarContas();
	signal(SIGUSR1, apanhaSinalSIGUSR1);

	printf("Bem-vinda/o ao i-banco\n\n");
	  
	while (1) {
		int numargs;
	
		numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);


		/* 
		 * Sair / Sair agora
		 * termina o programa
		 *
		 * no caso de EOF (end of file) do stdin ou COMANDO_SAIR
		 * arg1(optional) char* COMANDO_SAIR_AGORA
		 * com arg1 manda os processos filhos terminarem
		 * sem arg1 COMANDO_SAIR_AGORA espera que os filhos terminem  
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
			printf("--\n");

			if (numargs == 2 && (strcmp(args[1], COMANDO_SAIR_AGORA) == 0) ){	
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
			
			printf("--\n");
			printf("O i-banco terminou\n");
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
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
			   continue;
			}

			comando_t dualshock;

			dualshock.operacao = OPERACAO_DEBITAR;
			dualshock.idConta = atoi(args[1]);
			dualshock.valor = atoi(args[2]);

			addToBuffer(dualshock);

			/*
			figure out what to do with this
			
			if (debitar (idConta, valor) < 0)
			   printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, idConta, valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
			*/
		}

		/*
		 * Creditar 
		 * arg 1 int - idConta
		 * arg 2 int - valor
		 * credita ao idConta o valor
		 */
		else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
				continue;
			}

			comando_t dualshock;

			dualshock.operacao = OPERACAO_CREDITAR;
			dualshock.idConta = atoi(args[1]);
			dualshock.valor = atoi(args[2]);

			addToBuffer(dualshock);

			/* 
			figure out what to do with this

			if (creditar (idConta, valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
			*/
		}


		/*
		 * Ler Saldo
		 * arg 1 int - idConta
		 * le o saldo da conta idConta 
		 */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}
			
			comando_t dualshock;

			dualshock.operacao = OPERACAO_LER_SALDO;
			dualshock.idConta = atoi(args[1]);
						
			addToBuffer(dualshock);

			/* 
			figure out what to do with this

			if (saldo < 0)
				printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, idConta);
			else
				printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
			*/
		}

		


		/* 
		 * Simular 
		 * arg 1 int - nr_de_anos
		 * faz a simulacao dos nr_de_anos sob as contas existentes
		 */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
			
			if (numargs != 2 || atoi(args[1]) < 0) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
				continue;
			}

			comando_t dualshock;

			dualshock.operacao = OPERACAO_SIMULAR;

			addToBuffer(dualshock);
			
			int pid = fork();

			/* code for child process */
			if (pid == 0){
				simular(atoi(args[1]));
				exit(EXIT_SUCCESS);
			}

			pid_vector[process_counter] = pid;
			process_counter++;
		}


		else {
			printf("Comando desconhecido. Tente de novo.\n");
		}
	} 
}

void apanhaSinalSIGUSR1(){
	signal(SIGUSR1, apanhaSinalSIGUSR1);
	flag = 1;
}