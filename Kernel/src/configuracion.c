#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include "configuracion.h"
#include "estructuras.h"
#include "log.h"

extern char *ruta_config;
extern t_configuracion *config;
/*extern char *sem_id;
extern char *sem_in;
extern char *shared;*/

void leer_configuracion()
{
	escribir_log("Leyendo configuracion");
	char *ruta_aux = strdup(ruta_config);
	t_config *configuracion = config_create(ruta_aux);

	string_append(&config->algoritmo, config_get_string_value(configuracion,"ALGORITMO"));
	string_append(&config->ip_fs, config_get_string_value(configuracion, "IP_FS"));
	string_append(&config->ip_memoria, config_get_string_value(configuracion, "IP_MEMORIA"));
	string_append(&config->ip_kernel, config_get_string_value(configuracion, "IP_KERNEL"));
	config->sem_ids = config_get_array_value(configuracion, "SEM_IDS");
	config->sem_init = config_get_array_value(configuracion, "SEM_INIT");
	config->shared_vars = config_get_array_value(configuracion, "SHARED_VARS");
	config->stack_size = config_get_int_value(configuracion, "STACK_SIZE");
	config->grado_multiprog = config_get_int_value(configuracion, "GRADO_MULTIPROG");
	config->puerto_cpu = config_get_int_value(configuracion, "PUERTO_CPU");
	config->puerto_fs = config_get_int_value(configuracion, "PUERTO_FS");
	config->puerto_memoria = config_get_int_value(configuracion, "PUERTO_MEMORIA");
	config->puerto_prog = config_get_int_value(configuracion, "PUERTO_PROG");
	config->quantum = config_get_int_value(configuracion, "QUANTUM");
	config->quantum_sleep= config_get_int_value(configuracion, "QUANTUM_SLEEP");

	/*shared = strdup(config_get_string_value(configuracion, "SHARED_VARS"));
	sem_id = strdup(config_get_string_value(configuracion, "SEM_IDS"));
	sem_in = strdup(config_get_string_value(configuracion, "SEM_INIT"));*/

	config_destroy(configuracion);
	free(ruta_aux);
}
