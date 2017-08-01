/*
 * log.c
 *
 *  Created on: 29/11/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/string.h>

t_log *logi;

void crear_archivo_log(char *file)
{

	logi = log_create(file,"CPU",true, LOG_LEVEL_INFO);
	char *info = string_from_format("se crea archivo de log en ruta: %s",file);
	log_info(logi, info);
	free(info);
	free(file);
}

void escribir_log(char *mensaje,int cod)
{
	switch(cod){
	case 1:
		log_info(logi, mensaje);
		break;
	case 2:
		log_error(logi,mensaje);
		break;
	}

}

void liberar_log()
{
	log_destroy(logi);
}
