/* 
 * * * * * * * * * * * * *
 * Grupo: 4
 * Andre' Fonseca 84698
 * Isabel Dias 84726
 * 
 * * * * * * * * * * * * *
// Projeto SO - exercicio 2, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#include "inputProcessing.h"

/**
 * Initializes all necessary resources for i-banco to function with 
 * all features and calls the functions for it to work
 */



int main (int argc, char** argv) {

	int i;
	int status;

	signal(SIGUSR1, apanhaSinalSIGUSR1);
	

	/* Initializations */
	inicializarContas();

	/* initialize reading and writting semaphore writer_sem reading_sem*/
	if( sem_init(&writer_sem, 0, CMD_BUFFER_DIM) != 0 ||
		sem_init(&reader_sem, 0, 0) != 0){
		printf("Erro a inicializar semaforos, i-banco vai terminar.\n");
		exit(EXIT_FAILURE);
	}

	/*initialize reading_mutex*/
	if ( pthread_mutex_init(&reading_mutex, NULL) != 0){
		printf("Erro a criar semaforos, i-banco vai terminar.\n");
		exit(EXIT_FAILURE);
	}
	
	/* initialize mutex vector account_mutexes*/
	for(i = 0; i < NUM_CONTAS; i++){
		if ( pthread_mutex_init(account_mutexes + i, NULL) != 0){
			printf("Erro a inicializar mutexes, i-banco vai terminar.\n");
			exit(EXIT_FAILURE);			
		}
	}

	/* initialize thread_pool*/
	for(i = 0; i < NUM_TRABALHADORAS; i++){
		status = pthread_create(thread_pool + i, NULL, &readBuffer, NULL);
		if (status != 0){
			printf("Erro a criar threads, i-banco vai terminar.\n");
			exit(EXIT_FAILURE);
		}

	}

	if ( pthread_mutex_init(&active_commands_mutex, NULL) != 0){
		printf("Erro a inicializar mutex, i-banco vai terminar.\n");
		exit(EXIT_FAILURE);
	}

	if ( pthread_cond_init(&active_commands_cond, NULL) != 0){
		printf("Erro a inicializar cond, i-banco vai terminar.\n");
		exit(EXIT_FAILURE);
	}



	/* i-banco start */
	printf("Bem-vinda/o ao i-banco\n\n");

	processInput();

	return 0;
}
