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



extern int flag;


comando_t cmd_buffer[CMD_BUFFER_DIM];

pthread_t thread_pool[NUM_TRABALHADORAS];

pthread_mutex_t reading_mutex;

pthread_mutex_t account_mutexes[NUM_CONTAS];

pthread_mutex_t active_commands_mutex;

pthread_cond_t active_commands_cond;


sem_t writer_sem;

sem_t reader_sem;

int buff_write_idx;

int buff_read_idx;

int flag_sair;

int active_commands;

int log_file_descriptor;



/* comments/info located on .h file */


void apanhaSinalSIGUSR1(){
	signal(SIGUSR1, apanhaSinalSIGUSR1);
	flag = 1;
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

	char buffer[200];
	int size = snprintf(buffer, 50, "%lu: ", pthread_self());

	switch(comando.operacao){

		case OPERACAO_DEBITAR:

			pthread_mutex_lock(account_mutexes + (comando.idConta1 - 1));

			if (debitar (comando.idConta1, comando.valor) < 0) {
				printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
				size += snprintf(buffer + size, 50, "%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
			}
			else {
				printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
				size += snprintf(buffer + size, 50, "%s(%d, %d): OK\n\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
			}

			if (write(log_file_descriptor, buffer, size) == -1) 
				printf("Erro na escrita.");

			pthread_mutex_unlock(account_mutexes + (comando.idConta1 - 1));

			break;

		case OPERACAO_CREDITAR:

			pthread_mutex_lock(account_mutexes + (comando.idConta1 - 1));

			if (creditar (comando.idConta1, comando.valor) < 0) {
				printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
				size += snprintf(buffer + size, 50, "%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
			}
			else {
				printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
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
				printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, comando.idConta1);
				size += snprintf(buffer + size, 50, "%s(%d): Erro.\n\n", COMANDO_LER_SALDO, comando.idConta1);
			}
			else {
				printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, comando.idConta1, saldo);
				size += snprintf(buffer + size, 50, "%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, comando.idConta1, saldo);
			}

			if (write(log_file_descriptor, buffer, size) == -1) 
				perror("Erro na escrita.");

			pthread_mutex_unlock(account_mutexes + (comando.idConta1 - 1));

			break;

		case OPERACAO_SAIR:

			flag_sair = 1;
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
				printf("Erro ao transferir %d da conta %d para a conta %d\n\n", comando.valor, comando.idConta1, comando.idConta2);
				size += snprintf(buffer + size, 50, "Erro ao transferir %d da conta %d para a conta %d\n\n", comando.valor, comando.idConta1, comando.idConta2);
			}

			else {
				if (creditar(comando.idConta2, comando.valor) < 0){
					printf("Erro ao transferir %d da conta %d para a conta %d\n\n", comando.valor, comando.idConta1, comando.idConta2);
					size += snprintf(buffer + size, 50, "Erro ao transferir %d da conta %d para a conta %d\n\n", comando.valor, comando.idConta1, comando.idConta2);
				}
				else
					printf("%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, comando.idConta1, comando.idConta2, comando.valor);
					size += snprintf(buffer + size, 50, "%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, comando.idConta1, comando.idConta2, comando.valor);
			}
			
			if (write(log_file_descriptor, buffer, size) == -1) 
				perror("Erro na escrita.");

			pthread_mutex_unlock(account_mutexes + (comando.idConta1 - 1));
			pthread_mutex_unlock(account_mutexes + (comando.idConta2 - 1));

			break;


		default:
			printf("Comando Desconhecido.\n");
			break;
	}
}




void processInput(){

	buff_write_idx = 0;
	buff_read_idx = 0;
	flag_sair = 0;
	active_commands = 0;


	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE]; 

	int process_counter = 0; 
	int pid_vector[NR_MAX_PROCESSOS];

	while (1) {
		int numargs;
		numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);


		/* 
		 * Sair / Sair agora
		 * termina o programa apos todos os comandos  no cmd_buffer
		 * sejam concluidos pelos threads e que estes tenham terminado
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
			comando_t comando;

			if (numargs > 2 || numargs < 1) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIR);
				continue;
			}

			printf("O i-banco vai terminar\n");
			printf("--\n");

			comando.operacao = OPERACAO_SAIR;
			for ( i = 0; i < NUM_TRABALHADORAS ; i++)
				addToBuffer(comando);


			if (numargs == 2 && (strcmp(args[1], COMANDO_SAIR_AGORA) == 0) ){	
				for(i = 0; i < process_counter; i++)
					kill(pid_vector[i], SIGUSR1);
			} 

			
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
			return;
		}
	


		else if (numargs == 0)
			/* Nenhum argumento; ignora e volta a pedir */
			continue;
	

		/* 
		 * Debitar 
		 * arg 1 int - idConta1
		 * arg 2 int - valor
		 * adiciona a ao cmd_buffer a OPERACAO_DEBITAR  
		 */
		else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
			   continue;
			}

			comando_t comando;

			comando.operacao = OPERACAO_DEBITAR;
			comando.idConta1 = atoi(args[1]);
			comando.valor = atoi(args[2]);

			addToBuffer(comando);

		}

		/*
		 * Creditar 
		 * arg 1 int - idConta1
		 * arg 2 int - valor
		 * adiciona ao cmd_buffer a OPERACAO_CREDITAR
		 */
		else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
				continue;
			}

			comando_t comando;

			comando.operacao = OPERACAO_CREDITAR;
			comando.idConta1 = atoi(args[1]);
			comando.valor = atoi(args[2]);

			addToBuffer(comando);
		}


		/*
		 * Ler Saldo
		 * arg 1 int - idConta1
		 * adicionar ao cmd_buffer a OPERACAO_LER_SALDO
		 */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}
			
			comando_t comando;

			comando.operacao = OPERACAO_LER_SALDO;
			comando.idConta1 = atoi(args[1]);
						
			addToBuffer(comando);
		}

		

		/* 
		 * Simular 
		 * arg 1 int - nr_de_anos
		 * faz a simulacao dos nr_de_anos sob as contas existentes
		 *
		 * para nao ocorrer erros, espera que nao haja nenhuma thread a bloquear
		 */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
			
			if (numargs != 2 || atoi(args[1]) < 0) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
				continue;
			}

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
				
				simular(atoi(args[1]));

				close(file_descriptor);

				exit(EXIT_SUCCESS);
			}

			pid_vector[process_counter] = pid;
			process_counter++;
		}


		/*
		 * Transferir
		 * arg 1 int - idConta Origem
		 * arg 2 int - idConta Destino
		 * arg 3 int - valor 
		 * transfere valor de conta origem para conta destino
		 */
		else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0) {

			if (numargs != 4) {
				printf("%s: Sintaxe inválida, tente de novo. \n", COMANDO_TRANSFERIR);
				continue;
			}

			comando_t comando;

			int id1 = atoi(args[1]);
			int id2 = atoi(args[2]);
			int valor = atoi(args[3]);

			if (id1 == id2 || id1 < 0 || id2 < 0 || valor <= 0){
				printf("%s: Sintaxe inválida, tente de novo. \n", COMANDO_TRANSFERIR);
				continue;
			}

			comando.operacao = OPERACAO_TRANSFERIR;
			comando.idConta1 = id1;
			comando.idConta2 = id2;
			comando.valor = valor;

			addToBuffer(comando);
		}


		else {
			printf("Comando desconhecido. Tente de novo.\n");
		}
	}
}