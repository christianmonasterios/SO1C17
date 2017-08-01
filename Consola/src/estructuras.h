/*
 * estructuras.h
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
	char * ip;
	int puerto;
}t_consola;

typedef struct{
	int valor;
	sem_t val;
}t_chequeo;

typedef struct{
	int cantidad;
}t_impresiones;

typedef struct{
	pthread_t hilo;
}t_hilo;

typedef struct{
	time_t *tiempo;
	struct tm* tm_info;
	char buffer[26];
}t_tiempo;

typedef struct{
	int valor_chequeo;
	int cantidad_impresiones;
	int hilo;
	time_t *tiempoInicial;
}t_data;

#endif /* ESTRUCTURAS_H_ */
