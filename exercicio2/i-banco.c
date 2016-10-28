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
#include <semaphore.h>
#include <pthread.h>

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
#define OPERACAO_SAIR 777


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

void processCommand(comando_t dualshock);

/* flag changed by signal SIGUSR1 used in child process */
extern int flag;

comando_t cmd_buffer[CMD_BUFFER_DIM];

pthread_t thread_pool[NUM_TRABALHADORAS];

pthread_mutex_t reading_mutex;
pthread_mutex_t account_mutexes[NUM_CONTAS];

int accounts_in_use[NUM_TRABALHADORAS];
int buff_write_idx = 0;
int buff_read_idx = 0;
int flag_sair = 0;

sem_t writer_sem;
sem_t reader_sem;


void addToBuffer(comando_t dualshock){
	
	sem_wait(&writer_sem);
	cmd_buffer[buff_write_idx] = dualshock;
	buff_write_idx = (buff_write_idx + 1) % CMD_BUFFER_DIM;
	sem_post(&reader_sem);
}

void* readBuffer(){
	while (!flag_sair){
		comando_t dualshock;
		sem_wait(&reader_sem);
		pthread_mutex_lock(&reading_mutex);
		dualshock = cmd_buffer[buff_read_idx];
		buff_read_idx = (buff_read_idx + 1) % CMD_BUFFER_DIM;
		pthread_mutex_unlock(&reading_mutex);
		sem_post(&writer_sem);
		processCommand(dualshock);
	}
	pthread_exit(NULL);
	printf("terminou um thread\n");
	return NULL;
}

void initAccountMutexes() {
	int i;
	for(i = 0; i < NUM_CONTAS; i++)
		pthread_mutex_init(account_mutexes + i, NULL);
}

void processCommand(comando_t dualshock){

	int saldo;
	pthread_mutex_lock(account_mutexes + (dualshock.idConta - 1));

	switch(dualshock.operacao){

		case OPERACAO_DEBITAR:

			if (debitar (dualshock.idConta, dualshock.valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, dualshock.idConta, dualshock.valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, dualshock.idConta, dualshock.valor);

			break;

		case OPERACAO_CREDITAR:

			if (creditar (dualshock.idConta, dualshock.valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, dualshock.idConta, dualshock.valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, dualshock.idConta, dualshock.valor);

			break;

		case OPERACAO_LER_SALDO:

			saldo = lerSaldo(dualshock.idConta);

			if (saldo < 0)
				printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, dualshock.idConta);
			else
				printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, dualshock.idConta, saldo);

			break;

		case OPERACAO_SAIR:
			flag_sair = 1;
			break;

		default:
			printf("Comando Desconhecido.\n");
			break;
	}
	
	pthread_mutex_unlock(account_mutexes + (dualshock.idConta - 1));
}



int main (int argc, char** argv) {

	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE]; 

	int process_counter = 0; 
	int pid_vector[NR_MAX_PROCESSOS];
	int i;

	inicializarContas();
	signal(SIGUSR1, apanhaSinalSIGUSR1);

	sem_init(&writer_sem, 0, CMD_BUFFER_DIM);
	sem_init(&reader_sem, 0, 0);

	pthread_mutex_init(&reading_mutex, NULL);

	initAccountMutexes();

	for(i = 0; i < NUM_TRABALHADORAS; i++)
		pthread_create(thread_pool + i, NULL, &readBuffer, NULL);

	
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
			comando_t dualshock;

			if (numargs > 2 || numargs < 1) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIR);
				continue;
			}

			printf("O i-banco vai terminar\n");
			printf("--\n");

			dualshock.operacao = OPERACAO_SAIR;
			for ( i = 0; i < NUM_TRABALHADORAS ; i++)
				addToBuffer(dualshock);


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
			
			
			for ( i = 0; i < NUM_TRABALHADORAS ; i++){
				pthread_join(thread_pool[i], NULL);
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