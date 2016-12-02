
#include <stdio.h>
#include "commands.h"


comando_t cmd_buffer[CMD_BUFFER_DIM];

pthread_t thread_pool[NUM_TRABALHADORAS];

pthread_mutex_t reading_mutex;

pthread_mutex_t account_mutexes[NUM_CONTAS];

pthread_mutex_t active_commands_mutex;

pthread_cond_t active_commands_cond;


int pid_vector[20];

int process_counter;


sem_t writer_sem;

sem_t reader_sem;

int buff_write_idx;

int buff_read_idx;

int signal_flag;

int flag_sair;

int running_flag;

int active_commands;

int log_file_descriptor;

char pipe_name[100];



void initializeCommandResources(){

	process_counter = 0;

	int i;
	int status;

	signal(SIGUSR1, apanhaSinalSIGUSR1);
	

	/* Initializations */
	inicializarContas();

	/* initialize reading and writting semaphore writer_sem reading_sem*/
	if( sem_init(&writer_sem, 0, CMD_BUFFER_DIM) != 0 ||
		sem_init(&reader_sem, 0, 0) != 0){
		perror("Erro a inicializar semaforos, i-banco vai terminar.\n");
		exit(EXIT_FAILURE);
	}

	/*initialize reading_mutex*/
	if ( pthread_mutex_init(&reading_mutex, NULL) != 0){
		perror("Erro a criar mutex, i-banco vai terminar.\n");
		exit(EXIT_FAILURE);
	}
	
	/* initialize mutex vector account_mutexes*/
	for(i = 0; i < NUM_CONTAS; i++){
		if ( pthread_mutex_init(account_mutexes + i, NULL) != 0){
			perror("Erro a inicializar mutexes, i-banco vai terminar.\n");
			exit(EXIT_FAILURE);			
		}
	}

	

	/* initialize mutex responsible for active_commands access*/
	if ( pthread_mutex_init(&active_commands_mutex, NULL) != 0){
		perror("Erro a inicializar mutex, i-banco vai terminar.\n");
		exit(EXIT_FAILURE);
	}

	/* initialize condition variable fro active_commands */
	if ( pthread_cond_init(&active_commands_cond, NULL) != 0){
		perror("Erro a inicializar cond, i-banco vai terminar.\n");
		exit(EXIT_FAILURE);
	}


	/* open log file */
	log_file_descriptor = open("log.txt", O_RDWR|O_CREAT|O_APPEND, S_IRWXU);
	if (log_file_descriptor == -1){
		perror("erro ao abrir log.txt");

	}


	/* initialize thread_pool*/
	for(i = 0; i < NUM_TRABALHADORAS; i++){
		status = pthread_create(thread_pool + i, NULL, &readBuffer, NULL);
		if (status != 0){
			perror("Erro a criar threads, i-banco vai terminar.\n");
			exit(EXIT_FAILURE);
		}
	}
}


void closeCommandResources(){
	/* destruction of initialized mutexes, semaphores and so on */
	int i;
	sem_destroy(&writer_sem);
	sem_destroy(&reader_sem);
	pthread_mutex_destroy(&reading_mutex);

	for(i = 0; i < NUM_CONTAS; i++)
		pthread_mutex_destroy(account_mutexes + i);

	pthread_mutex_destroy(&active_commands_mutex);
	pthread_cond_destroy(&active_commands_cond);

	close(log_file_descriptor);
}



void readCommand(comando_t comando) {

	if (comando.operacao == OPERACAO_SIMULAR){

		pthread_mutex_lock(&active_commands_mutex);

		while (active_commands != 0)
			pthread_cond_wait(&active_commands_cond, &active_commands_mutex);

		pthread_mutex_unlock(&active_commands_mutex);
		
		
		int pid = fork();

		/* code for child process */
		if (pid == 0){
			char name[100];
			snprintf(name, 50, "i-banco-sim-%d.txt", getpid());

			int file_descriptor = open(name, O_RDWR|O_CREAT|O_APPEND, S_IRWXU);
			if (file_descriptor == -1)
				perror("erro ao abrir i-banco-sim.");

			close(1);
			dup(file_descriptor);
			simular(comando.valor);
			close(file_descriptor);
			exit(EXIT_SUCCESS);
		}

		pid_vector[process_counter] = pid;
		process_counter++;
	}


	else if (comando.operacao == OPERACAO_SAIR || comando.operacao == OPERACAO_SAIR_AGORA){
		int i;
		int status;
		int pid;

		printf("O i-banco vai terminar\n");
		printf("--\n");

		for ( i = 0; i < NUM_TRABALHADORAS ; i++)
			addToBuffer(comando);

		if (comando.operacao == OPERACAO_SAIR_AGORA)
			for(i = 0; i < process_counter; i++)
				kill(pid_vector[i], SIGUSR1);

		/* waits for all child processes to end */
		for (i = 0; i < process_counter; i++){
			pid = wait (&status);
			
			if (WIFEXITED(status))
				printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n", pid);

			else 
				printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n", pid);
		}
		
		
		/*  waits for all threads to end, 
			for simplicity no end status is checked  */
		for ( i = 0; i < NUM_TRABALHADORAS ; i++){
			pthread_join(thread_pool[i], NULL);
		}

		printf("--\n");
		printf("O i-banco terminou\n");
		running_flag = 0; 

	}
	else{ 
		addToBuffer(comando);
	}
}








void addToBuffer(comando_t comando){
	
	sem_wait(&writer_sem);

	cmd_buffer[buff_write_idx] = comando;
	buff_write_idx = (buff_write_idx + 1) % CMD_BUFFER_DIM;
	
	sem_post(&reader_sem);
}




void* readBuffer(){
	comando_t comando;

	while (!flag_sair){
		sem_wait(&reader_sem);
		
		/* starting a command */
		pthread_mutex_lock(&active_commands_mutex);
		active_commands++;
		pthread_mutex_unlock(&active_commands_mutex);

		/* accessing cmd_buffer related variable */
		pthread_mutex_lock(&reading_mutex);
		comando = cmd_buffer[buff_read_idx];
		buff_read_idx = (buff_read_idx + 1) % CMD_BUFFER_DIM;
		pthread_mutex_unlock(&reading_mutex);
		
		sem_post(&writer_sem);
		

		processCommand(comando);


		/* finished processing command */
		pthread_mutex_lock(&active_commands_mutex);
		active_commands--;
		pthread_mutex_unlock(&active_commands_mutex);
		pthread_cond_signal(&active_commands_cond);
	}
	
	pthread_exit(NULL); /* for simplicity we won't give exit status */
	printf("terminou um thread\n");
	
	return NULL; /* for simplicity we won't give exit status */
}








void processCommand(comando_t comando){


	if(comando.operacao == OPERACAO_SAIR || comando.operacao == OPERACAO_SAIR_AGORA){
		flag_sair = 1;
		return;
	}

	char buffer[400];
	char outputPipe[50];
	int size = snprintf(buffer, 50, "%lu: ", pthread_self());
	int initial_size = size;
	snprintf(outputPipe, 50, "%d", comando.pid);

	int temp_pipe = open(outputPipe ,O_WRONLY);
	time_t time1;
	time_t time2;

	time(&time1);

	switch(comando.operacao){

		case OPERACAO_DEBITAR:

			pthread_mutex_lock(account_mutexes + (comando.idConta1 - 1));

			if (debitar (comando.idConta1, comando.valor) < 0) {
				size += snprintf(buffer + size, 50, "%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
			}
			else {
				size += snprintf(buffer + size, 50, "%s(%d, %d): OK\n\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
			}

			if (write(log_file_descriptor, buffer, size) == -1) 
				printf("Erro na escrita.");

			pthread_mutex_unlock(account_mutexes + (comando.idConta1 - 1));

			break;

		case OPERACAO_CREDITAR:

			pthread_mutex_lock(account_mutexes + (comando.idConta1 - 1));

			if (creditar (comando.idConta1, comando.valor) < 0) {
				size += snprintf(buffer + size, 50, "%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
			}
			else {
				size += snprintf(buffer + size, 50, "%s(%d, %d): OK\n\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
			}

			if (write(log_file_descriptor, buffer, size) == -1)
				perror("Erro na escrita.");

			pthread_mutex_unlock(account_mutexes + (comando.idConta1 - 1));

			break;

		case OPERACAO_LER_SALDO:

			pthread_mutex_lock(account_mutexes + (comando.idConta1 - 1));

			int saldo = lerSaldo(comando.idConta1);

			if (saldo < 0) {
				size += snprintf(buffer + size, 50, "%s(%d): Erro.\n\n", COMANDO_LER_SALDO, comando.idConta1);
			}
			else {
				size += snprintf(buffer + size, 50, "%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, comando.idConta1, saldo);
			}

			if (write(log_file_descriptor, buffer, size) == -1) 
				perror("Erro na escrita.");

			pthread_mutex_unlock(account_mutexes + (comando.idConta1 - 1));

			break;



		case OPERACAO_TRANSFERIR:
			/* chosen lock order by smallest first to avoid deadlock */
			if (comando.idConta1 < comando.idConta2) {
				pthread_mutex_lock(account_mutexes + (comando.idConta1 - 1));
				pthread_mutex_lock(account_mutexes + (comando.idConta2 - 1));
			}
			else {
				pthread_mutex_lock(account_mutexes + (comando.idConta2 - 1));
				pthread_mutex_lock(account_mutexes + (comando.idConta1 - 1));
			}

			if (debitar(comando.idConta1, comando.valor) < 0){
				size += snprintf(buffer + size, 50, "Erro ao transferir %d da conta %d para a conta %d\n\n", comando.valor, comando.idConta1, comando.idConta2);
			}

			else {
				if (creditar(comando.idConta2, comando.valor) < 0){
					size += snprintf(buffer + size, 50, "Erro ao transferir %d da conta %d para a conta %d\n\n", comando.valor, comando.idConta1, comando.idConta2);
				}
				else{
					size += snprintf(buffer + size, 50, "%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, comando.idConta1, comando.idConta2, comando.valor);
				}
			}
			
			if (write(log_file_descriptor, buffer, size) == -1) 
				perror("Erro na escrita.");

			pthread_mutex_unlock(account_mutexes + (comando.idConta1 - 1));
			pthread_mutex_unlock(account_mutexes + (comando.idConta2 - 1));

			break;


		default:
			printf("Comando Desconhecido.\n");
			return;
	}

	time(&time2);
	size += snprintf(buffer + size-1, 200, "\toperation time: %lf \n", difftime(time2,time1));
	if (write(temp_pipe, buffer+initial_size, size-initial_size) == -1) 
		printf("Error on delivering operation result");
}



/* comments/info located on .h file */

void apanhaSinalSIGUSR1(){
	signal(SIGUSR1, apanhaSinalSIGUSR1);
	signal_flag = 1;
}
