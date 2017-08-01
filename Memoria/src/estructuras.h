/*
 * estructuras.h
 *
 *  Created on: 2/4/2017
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

//Estructura definida para el modulo de memoria
typedef struct{
	char *IP;
	int PUERTO;
	int MARCOS;
	int MARCO_SIZE;
	int ENTRADAS_CACHE;
	int CACHE_X_PROC;
	int REEMPLAZO_CACHE;
	int RETARDO_MEMORIA;
	int SOCKET;
}t_memoria;

typedef struct{
	int estado;
	int pid;
	int pag;
} t_tablaPagina;


typedef struct{
	int pid;
	int pag;
	int LRU;
}t_LRU_cache;

typedef struct{
	int pid;
	int pag;
	char * dataFrame;
} t_cache;

typedef struct{
	int pid;
	int cantPaginas;
	int ultimaPagAsig;
}t_proceso;

#endif /* ESTRUCTURAS_H_ */
