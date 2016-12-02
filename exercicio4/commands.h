
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>


/* commands */
#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_SAIR_AGORA "agora"
#define COMANDO_TRANSFERIR "transferir"
#define COMANDO_SAIR_TERMINAL "sair-terminal"

/* operations related to the commands */
#define OPERACAO_DEBITAR 1337
#define OPERACAO_CREDITAR 666
#define OPERACAO_LER_SALDO 007
#define OPERACAO_SIMULAR 2319
#define OPERACAO_SAIR 112
#define OPERACAO_TRANSFERIR 11092016
#define OPERACAO_SAIR_AGORA 4582
 

/* temporary input buffer related defines*/ 
#define MAXARGS 4
#define BUFFER_SIZE 100

/* threads and command related defines */ 
#define NR_MAX_PROCESSOS 20
#define NUM_TRABALHADORAS 3
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)


/* command structure */
typedef struct {
	int pid;
	int operacao;
	int idConta1;
	int idConta2;
	int valor;
}comando_t;


/* buffer with all given commands produced by input and consumed by threads */
extern comando_t cmd_buffer[CMD_BUFFER_DIM];

/* vector with all our threads aka pool*/
extern pthread_t thread_pool[NUM_TRABALHADORAS];

/* mutex to limit the access of the cmd buffer to 1 thread */
extern pthread_mutex_t reading_mutex;

/* mutex vector to limit the access to a single thread per account*/
extern pthread_mutex_t account_mutexes[NUM_CONTAS];

/* mutex used for accessing active_commands global variable */
extern pthread_mutex_t active_commands_mutex;

/* cond for the condition variable active command */
extern pthread_cond_t active_commands_cond;

/* semaphore that controls the insertion of new commands on cmd buffer */
extern sem_t writer_sem;

/* semaphore that controls the consuption of new commands on cmd buffer */
extern sem_t reader_sem;

/* tracks cmd buffer's next writting position*/
extern int buff_write_idx;

/* tracks cmd buffer's next reading position */
extern int buff_read_idx;

/* file descriptor that logs all command operations */
extern int log_file_descriptor;



void initializeCommandResources();

void closeCommandResources();


void apanhaSinalSIGUSR1();
/** 
 * function that processes signal SIGUSR1 on child processes
 */



void addToBuffer(comando_t comando);
/**
 * adds to cmd buffer the given comando-t (command) if 
 * there is room in buffer for it, else it waits until 
 * it is able to add to buffer
 * @param comando command to be written on cmd buffer
 */

void* readBuffer();
/**
 * Continuously reads and processes commands from buffer 
 * until a specific command is given to stop.
 *
 * Used by threads to consume and process from the buffer 
 * while there are commands left to be processed.
 */

void processCommand(comando_t comando);
/**
 * Processes/executes the given command inside in a way
 * that limits one thread per account ID
 * 
 * arg: comand_t comando
 */

void processInput();
/**
 * continuously asks for input until stop command is given
 * converts the program's input into commands (comando_t)
 * verifies basic command realted info before creating command
 * 
 */

void readCommand(comando_t comando);
