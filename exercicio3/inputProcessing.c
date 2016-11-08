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


sem_t writer_sem;

sem_t reader_sem;

int buff_write_idx;

int buff_read_idx;

int flag_sair;





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
		
		pthread_mutex_lock(&reading_mutex);
		
		/* access to shared resources */
		comando = cmd_buffer[buff_read_idx];
		buff_read_idx = (buff_read_idx + 1) % CMD_BUFFER_DIM;
		/* end of access to shared resources */
		
		pthread_mutex_unlock(&reading_mutex);
		
		sem_post(&writer_sem);
		

		processCommand(comando);
	}
	
	pthread_exit(NULL); /* for simplicity we won't give exit status*/
	printf("terminou um thread\n");
	
	return NULL; /* for simplicity we won't give exit status */
}








void processCommand(comando_t comando){

	int saldo;
	pthread_mutex_lock(account_mutexes + (comando.idConta - 1));

	switch(comando.operacao){

		case OPERACAO_DEBITAR:

			if (debitar (comando.idConta, comando.valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, comando.idConta, comando.valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, comando.idConta, comando.valor);

			break;

		case OPERACAO_CREDITAR:

			if (creditar (comando.idConta, comando.valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, comando.idConta, comando.valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, comando.idConta, comando.valor);

			break;

		case OPERACAO_LER_SALDO:

			saldo = lerSaldo(comando.idConta);

			if (saldo < 0)
				printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, comando.idConta);
			else
				printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, comando.idConta, saldo);

			break;

		case OPERACAO_SAIR:
			flag_sair = 1;
			break;

		default:
			printf("Comando Desconhecido.\n");
			break;
	}
	
	pthread_mutex_unlock(account_mutexes + (comando.idConta - 1));
}




void processInput(){

	buff_write_idx = 0;
	buff_read_idx = 0;
	flag_sair = 0;

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
			exit(EXIT_SUCCESS);
		}
	


		else if (numargs == 0)
			/* Nenhum argumento; ignora e volta a pedir */
			continue;
	

		/* 
		 * Debitar 
		 * arg 1 int - idConta
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
			comando.idConta = atoi(args[1]);
			comando.valor = atoi(args[2]);

			addToBuffer(comando);

		}

		/*
		 * Creditar 
		 * arg 1 int - idConta
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
			comando.idConta = atoi(args[1]);
			comando.valor = atoi(args[2]);

			addToBuffer(comando);
		}


		/*
		 * Ler Saldo
		 * arg 1 int - idConta
		 * adicionar ao cmd_buffer a OPERACAO_LER_SALDO
		 */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}
			
			comando_t comando;

			comando.operacao = OPERACAO_LER_SALDO;
			comando.idConta = atoi(args[1]);
						
			addToBuffer(comando);
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

			comando_t comando;

			comando.operacao = OPERACAO_SIMULAR;

			addToBuffer(comando);
			
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