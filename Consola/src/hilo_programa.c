/*
 * hilo_programa.c
 *
 *  Created on: 6/5/2017
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include "socket_client.h"
#include "mensaje.h"
#include "log.h"
#include "estructuras.h"

extern t_dictionary * sem;
extern sem_t semaforo;
extern sem_t x;
time_t *tiempoInicial;
extern t_dictionary * tiempo;
extern t_dictionary * impresiones;
extern char *aImprimir;

void programa(char* pid);

void programa(char* pid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	t_chequeo *semp;
	t_impresiones *impresiones2;
	t_tiempo * tiempo2;

	tiempo2 = malloc(sizeof(t_tiempo));
	tiempoInicial = malloc(sizeof(time_t));
	tiempo2->tiempo = malloc(sizeof(time_t));

	*tiempoInicial = time(NULL);
	tiempo2->tm_info = localtime(tiempoInicial);
	strftime(tiempo2->buffer, 26, "%Y-%m-%d %H:%M:%S", tiempo2->tm_info);

	tiempo2->tiempo = tiempoInicial;
	dictionary_put(tiempo,pid,tiempo2);

	impresiones2 = dictionary_get(impresiones,pid);
	semp = dictionary_get(sem,pid);

	while(1)
	{
		//if(semp->valor==1)
		//{
		sem_wait(&semp->val);
		sem_wait(&semaforo);

		escribir_log(aImprimir);
		printf("%s\n", aImprimir);

		impresiones2->cantidad++;
		escribir_log_con_numero("Cantidad impresiones: ", impresiones2->cantidad);
		free(aImprimir);
		semp->valor = 0;
		sem_post(&semaforo);
		sem_post(&x);
		//sem_post(&semp->val);
		//}
	}
	free(tiempoInicial);
	free(tiempo2);
}
