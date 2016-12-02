#include <stdio.h>
#include "commands.h"



/**
 * GRUPO 4
 * Andre Fonseca 84698
 * Isabel Dias 84726
 * Projecto de SO 
 * exercicio 4
 */

/* ibanco server pipe */
int i_banco_pipe;

/* name for the response pipe */
char response_pipe_name[NAME_SIZE];

/* activated on attempt to write on broken pipe */
int flag_closed_pipe;


void addToPipe(comando_t comando);
void apanhaSinalSIGPIPE();
void processInput();



/**
 * receives input from stdin, generates valid commands and sends
 * to the server pipe (if no pipe name is given, default will be opened)
 * @param  argc number of arguments main receives
 * @param  argv vector of string arguments
 * @return      status
 */
int main(int argc, char const *argv[]){
	
	flag_closed_pipe = 0;
	char i_banco_pipe_name[NAME_SIZE];
	signal(SIGPIPE, apanhaSinalSIGPIPE);


	/* decides target pipe name */
	if (argc == 1)
		strcpy(i_banco_pipe_name, "i-banco-pipe");
	else if(argc == 2)
		strcpy(i_banco_pipe_name, argv[1]);
	else{
		printf("Bad argument input\n");
		return 0;
	}


	i_banco_pipe = open(i_banco_pipe_name,O_WRONLY);	
	if (i_banco_pipe == -1)
		perror("Initialization - Erro a abrir o pipe do server");

	printf("- Connection to Server Successful -\n\n");

	/* create response pipe name with pid */
	snprintf(response_pipe_name, NAME_SIZE, "%d", getpid());


	/*Creating response pipe */
	if(unlink(response_pipe_name))
		if(errno != ENOENT)
			perror(" initialization - Erro no unlink do response pipe");

	if (mkfifo(response_pipe_name, S_IRWXU) != 0)
		perror("initialization - Erro a abrir o pipe terminal.");


	/* starts the processing 
	loop of received input */
	printf("Bem vinda/o ao i-banco-terminal.\n\n");
	processInput();
	

	if(close(i_banco_pipe))
		perror("Erro no close do default pipe");

	if(unlink(response_pipe_name))
		if(errno != ENOENT)
			perror("Erro no unlink do response pipe");
	
	return 0;
}





/**
 * Send to server pipe the given command and if necessary
 * awaits for response from response pipe and prints response
 * @param comando to be sent to pipe
 */
void addToPipe(comando_t comando){
	
	comando.pid = getpid();

	if (write(i_banco_pipe, &comando, sizeof(comando_t)) == -1)
		perror("Erro na escrita no i-banco-pipe.");

	/* operators that do not need response */
	if( comando.operacao == OPERACAO_SAIR_AGORA ||
		comando.operacao == OPERACAO_SAIR ||
		comando.operacao == OPERACAO_SIMULAR)
		return;

	
	int fd = open(response_pipe_name,O_RDONLY);
	
	if (fd < 0){
		perror("Erro ao abrir o pipe de retorno.");
		return;
	}

	/* prints response from server (response pipe)*/
	char buffer[NAME_SIZE];
	
	if(read(fd, buffer, NAME_SIZE) < 0)
		perror("addToPipe - erro ao ler do response-pipe");
	
	printf("%s\n", buffer);
	
	if(close(fd) < 0)
		perror("addToPipe - erro ao fechar o response-pipe");
}







void processInput(){

	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE]; 

	while (1) {

		if (flag_closed_pipe){
			printf("Connection with server lost, please shutdown with command \"sair-terminal\".\n");
		}

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
			
			comando_t comando;

			if (numargs > 2 || numargs < 1) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIR);
				continue;
			}

			comando.operacao = OPERACAO_SAIR;


			if (numargs == 2 && (strcmp(args[1], COMANDO_SAIR_AGORA) == 0) ){	
				comando.operacao = OPERACAO_SAIR_AGORA;
			} 

			addToPipe(comando);
		}

		else if (numargs < 0 || 
				(numargs > 0 && 
				(strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0 ))) {

			printf("i-banco-terminal vai terminar.\n");
			sleep(2);
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

			addToPipe(comando);
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

			addToPipe(comando);
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

			addToPipe(comando);
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
			comando_t comando;

			comando.operacao = OPERACAO_SIMULAR;
			comando.valor = atoi(args[1]);

			addToPipe(comando);
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

			addToPipe(comando);
		}


		else {
			printf("Comando desconhecido. Tente de novo.\n");
		}
	}
}




void apanhaSinalSIGPIPE(){
	signal(SIGPIPE, apanhaSinalSIGPIPE);
	flag_closed_pipe = 1;
}
