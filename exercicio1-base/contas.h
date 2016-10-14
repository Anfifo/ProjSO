/*
 * * * * * * * * * * * * *
 * Grupo: 4
 * Andre' Fonseca 84698
 * Isabel Dias 84726
 *
 * added/edited:
 * 		simular
 * * * * * * * * * * * * *
// Operações sobre contas, versao 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#ifndef CONTAS_H
#define CONTAS_H

#define NUM_CONTAS 10
#define TAXAJURO 0.1
#define CUSTOMANUTENCAO 1

#define ATRASO 1

void inicializarContas();
int contaExiste(int idConta);
int debitar(int idConta, int valor);
int creditar(int idConta, int valor);
int lerSaldo(int idConta);

/* 
 * WARNING (simular):
 * This function expects to be called on a child process
 * it directly access the vector contasSaldos and changes it
 * calling this function on parents process will make unwanted changes to the vector
 *
 * simulates all account's balance for the next numAnos years according to formula
 */
void simular(int numAnos);



#endif
