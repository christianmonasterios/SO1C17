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

extern t_log *log_;
pthread_mutex_t mutex_log;

void crear_archivo_log(char *file)
{
	log_ = log_create(file,"MEMORIA",false, LOG_LEVEL_INFO);
	log_info(log_, "Se crea el archivo de log Memoria");
	pthread_mutex_init(&mutex_log,NULL);
}

void escribir_log(char *mensaje)
{
	pthread_mutex_lock(&mutex_log);
	log_info(log_, mensaje);
	pthread_mutex_unlock(&mutex_log);
}

void escribir_error_log(char *mensaje)
{
	pthread_mutex_lock(&mutex_log);
	log_error(log_, mensaje);
	pthread_mutex_unlock(&mutex_log);
}

void escribir_log_con_numero(char *mensaje, int un_numero)
{
	char *final = strdup(mensaje);
	char *num = string_itoa(un_numero);
	string_append(&final, num);
	pthread_mutex_lock(&mutex_log);
	log_info(log_, final);
	pthread_mutex_unlock(&mutex_log);
	free(final);
	free(num);
}

void escribir_log_compuesto(char *mensaje, char *otro_mensaje)
{
	char *final = strdup("");
	string_append(&final, mensaje);
	string_append(&final, otro_mensaje);
	pthread_mutex_lock(&mutex_log);
	log_info(log_, final);
	pthread_mutex_unlock(&mutex_log);
	free(final);
}

void escribir_log_error_compuesto(char *mensaje, char *otro_mensaje)
{
	char *final = strdup("");
	string_append(&final, mensaje);
	string_append(&final, otro_mensaje);
	pthread_mutex_lock(&mutex_log);
	log_error(log_, final);
	pthread_mutex_unlock(&mutex_log);
	free(final);
}

void liberar_log()
{
	log_destroy(log_);
}
