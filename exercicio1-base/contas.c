#include "contas.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define atrasar() sleep(ATRASO)


int contasSaldos[NUM_CONTAS];
int flag;


int contaExiste(int idConta) {
	return (idConta > 0 && idConta <= NUM_CONTAS);
}

void inicializarContas() {
	int i;
	for (i=0; i<NUM_CONTAS; i++)
		contasSaldos[i] = 0;
}

int debitar(int idConta, int valor) {
	atrasar();
	if (!contaExiste(idConta))
		return -1;
	if (contasSaldos[idConta - 1] < valor)
		return -1;
	atrasar();
	contasSaldos[idConta - 1] -= valor;
	return 0;
}

int creditar(int idConta, int valor) {
	atrasar();
	if (!contaExiste(idConta))
		return -1;
	contasSaldos[idConta - 1] += valor;
	return 0;
}

int lerSaldo(int idConta) {
	atrasar();
	if (!contaExiste(idConta))
		return -1;
	return contasSaldos[idConta - 1];
}



/* 
 * WARNING:
 * This function expects to be called on a child process
 * it directly access the vector contasSaldos and changes it
 * calling this function on parents process will make unwanted changes to the vector
 *
 * since we're using child processes to deal with this function it makes no sense
 * to create an auxiliar vector on this function
 */
void simular(int numAnos) {

	int ano;
	int idConta;
	flag = 0;


	if (numAnos < 0)
		return;


	for (ano = 0; ano <= numAnos; ano++){
		printf("SIMULACAO: Ano: %d \n", ano);
		printf("=============\n");
		
		for (idConta = 1; idConta <= NUM_CONTAS; idConta++){
			
			if (!contaExiste(idConta))
				continue;

			if (ano > 0)
				contasSaldos[idConta-1] = (int) ((float) lerSaldo(idConta) * (1 + TAXAJURO) - (float)CUSTOMANUTENCAO);

			if (contasSaldos[idConta-1] < 0)
				contasSaldos[idConta-1] = 0;

			printf("Conta %d, Saldo %d\n", idConta, contasSaldos[idConta-1]);
		}

		printf("\n");
		
		/* se um sinal tornar a flag a 1 o processo 
		 * termina no final da simulacao do ano */ 
		if (flag){
			printf("terminado por sinal\n");
			exit(EXIT_SUCCESS);
		}
	}
}








