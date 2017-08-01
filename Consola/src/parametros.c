/*
 * parametros.c
 *
 *  Created on: 30/4/2017
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estructuras.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include "log.h"
#include <commons/log.h>

extern t_dictionary * p_pid;
extern t_dictionary * h_pid;
extern t_dictionary * sem;
extern t_dictionary * tiempo;
extern t_dictionary * impresiones;
extern t_dictionary * procesos;
extern t_dictionary *no_iniciados;
extern t_consola * arch_config;


void inicializar_parametros()
{
	arch_config = malloc(sizeof(t_consola));
	arch_config->ip= strdup("");
	p_pid = dictionary_create();
	h_pid = dictionary_create();
	procesos = dictionary_create();
	sem = dictionary_create();
	impresiones = dictionary_create();
	tiempo = dictionary_create();
	no_iniciados = dictionary_create();
}

void liberar_memoria()
{
	free(arch_config->ip);
	dictionary_clean(p_pid);
	dictionary_destroy(p_pid);
	dictionary_clean(h_pid);
	dictionary_destroy(h_pid);
	dictionary_clean(sem);
	dictionary_destroy(sem);
	dictionary_clean(tiempo);
	dictionary_destroy(tiempo);
	dictionary_clean(impresiones);
	dictionary_destroy(impresiones);
	dictionary_clean(procesos);
	dictionary_destroy(procesos);
	dictionary_clean(no_iniciados);
	dictionary_destroy(no_iniciados);
	free(arch_config);
	liberar_log();
}


