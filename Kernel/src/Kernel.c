#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include "configuracion.h"
#include "manejo_errores.h"
#include "consolaKernel.h"
#include "manejo_conexiones.h"
#include "semaforos_vglobales.h"
#include "planificador.h"
#include "estructuras.h"
#include "cpuManager.h"
#include "memoria.h"
#include "mensaje.h"
#include "socket.h"
#include "log.h"

char *ruta_config;
t_configuracion *config;
char *sem_id;
char *sem_in;
char *shared;
char *offs;
t_dictionary *sems;
t_dictionary *vglobales;
t_list *list_cpus;
t_list *list_consolas;
t_list *list_ejecutando;
t_list *list_finalizados;
t_list *list_bloqueados;
t_list *global_fd;
t_queue *cola_nuevos;
t_queue *cola_listos;
t_log *log_;
pthread_mutex_t mutex_lista_cpus;
pthread_mutex_t mutex_lista_fs;
pthread_mutex_t mutex_lista_consolas;
pthread_mutex_t mutex_lista_ejecutando;
pthread_mutex_t mutex_lista_finalizados;
pthread_mutex_t mutex_lista_bloqueados;
pthread_mutex_t mutex_cola_nuevos;
pthread_mutex_t mutex_cola_listos;
pthread_mutex_t mutex_actualizar_multip;
pthread_mutex_t mutex_planificar;
pthread_mutex_t mutex_socket;
pthread_mutex_t mutex_sincro;
sem_t sem_grad_multi;
sem_t sem_nuevos;
sem_t sem_listos;
sem_t sem_cpus;
int flag_planificador = 0;
int ultimo_pid = 1;
int tam_pagina = 0;
int pag_cod = 0;
int pag_stack = 0;
int diferencia_multi = 0;
int fd_inotify;
int watch_descriptor;

#define PAGE_SIZE 4096
#define STK_SIZE (10 * PAGE_SIZE)

void inicializar_variables();
void liberar_memoria();
void handshakearFS();
void crear_conexiones();
void inicializar_semaforos();
void programas_listos_A_ejecutar();
void programas_nuevos_A_listos();
void manejo_conexiones();
void iniciar_monitoreo_configuracion(char *);

int main(int argc, char*argv[])
{
	pthread_t hiloConsolaConsola;
	pthread_t update_multiprogramacion;
	pthread_t hiloNuevos;
	pthread_t hiloListos;
	pthread_t hiloConsolaKernel;

	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr,STK_SIZE);

	ruta_config = strdup(argv[1]);

	crear_archivo_log("/home/utnso/log_kernel.txt");
	inicializar_variables();
	leer_configuracion();
	inicializar_semaforos();

	inicializar_sems();
	inicializar_vglobales();

	iniciar_monitoreo_configuracion(ruta_config);

	crear_conexiones();
	handshakearMemory();
	handshakearFS();

	pthread_create(&hiloConsolaConsola, &attr, (void*)manejo_conexiones, NULL);
	pthread_create(&hiloNuevos, NULL, (void*)programas_nuevos_A_listos, NULL);
	pthread_create(&hiloListos, NULL, (void*)programas_listos_A_ejecutar, NULL);
	pthread_create(&hiloConsolaKernel, NULL, (void*)leer_consola, NULL);
	pthread_create(&update_multiprogramacion, NULL, (void*)actualizar_grado_multiprogramacion, NULL);

	pthread_join(hiloConsolaConsola, NULL);
	pthread_join(hiloNuevos, NULL);
	pthread_join(hiloListos, NULL);
	pthread_join(hiloConsolaKernel, NULL);
	pthread_join(update_multiprogramacion, NULL);

	liberar_memoria();
	return EXIT_SUCCESS;
}

void inicializar_variables()
{
	escribir_log("Inicializar variables");
	config = malloc(sizeof(t_configuracion));
	config->algoritmo = strdup("");
	config->ip_fs = strdup("");
	config->ip_memoria = strdup("");
	config->ip_kernel = strdup("");
	/*config->sem_ids = list_create();
	config->sem_init = list_create();
	config->shared_vars = list_create();*/
	global_fd = list_create();
	list_cpus = list_create();
	list_consolas = list_create();
	list_ejecutando = list_create();
	list_finalizados = list_create();
	cola_nuevos = queue_create();
	cola_listos = queue_create();
	list_bloqueados = list_create();
}

void liberar_memoria()
{
	escribir_log("Liberando memoria");
	free(config->algoritmo);
	free(config->ip_fs);
	free(config->ip_memoria);
	free(config->ip_kernel);
	free(config);
	/*list_destroy(config->sem_ids);
	list_destroy(config->sem_init);
	list_destroy(config->shared_vars);
	list_destroy_and_destroy_elements(cpus, void(*element_destroyer)(void*));
	list_destroy_and_destroy_elements(consolas, void(*element_destroyer)(void*));	*/
}

void handshakearFS()
{
	escribir_log("Realizando handshake con FS");
	int controlador = 0;
	char *mensaje = armar_mensaje("K03","");
	enviar(config->cliente_fs, mensaje, &controlador);
	free(mensaje);
}

void crear_conexiones()
{
	int controlador = 0;
	escribir_log("Creando server Kernel - Cpu");
	config->server_cpu = iniciar_socket_server(config->ip_kernel, config->puerto_cpu, &controlador);
	escribir_log("Creando server Kernel - Consola");
	config->server_consola = iniciar_socket_server(config->ip_kernel, config->puerto_prog, &controlador);
	escribir_log("Conectando con FS");
	config->cliente_fs = iniciar_socket_cliente(config->ip_fs, config->puerto_fs, &controlador);
	escribir_log("Conectando con Memoria");
	config->cliente_memoria = iniciar_socket_cliente(config->ip_memoria, config->puerto_memoria, &controlador);
}

void inicializar_semaforos()
{
	escribir_log("Inicializando semaforos");
	pthread_mutex_init(&mutex_lista_cpus,NULL);
	pthread_mutex_init(&mutex_lista_consolas,NULL);
	pthread_mutex_init(&mutex_lista_ejecutando,NULL);
	pthread_mutex_init(&mutex_lista_finalizados,NULL);
	pthread_mutex_init(&mutex_lista_bloqueados,NULL);
	pthread_mutex_init(&mutex_cola_nuevos,NULL);
	pthread_mutex_init(&mutex_cola_listos,NULL);
	pthread_mutex_init(&mutex_lista_fs,NULL);
	pthread_mutex_init(&mutex_actualizar_multip,NULL);
	pthread_mutex_init(&mutex_planificar,NULL);
	pthread_mutex_init(&mutex_socket,NULL);
	pthread_mutex_init(&mutex_sincro,NULL);

	sem_init(&sem_nuevos,0,0);
	sem_init(&sem_listos,0,0);
	sem_init(&sem_cpus,0,0);
	if(sem_init(&sem_grad_multi,0,config->grado_multiprog) != 0)
	{
		escribir_log_error_con_numero("Error asignando el grado de multiprogramacion: ", config->grado_multiprog);
	}
}
void iniciar_monitoreo_configuracion(char *path)
{
	fd_inotify = inotify_init();
	if (fd_inotify < 0) {
		perror("inotify_init");
	}
	char  *directorio_config;
	directorio_config = sacar_nombre_archivo(path);
	watch_descriptor = inotify_add_watch(fd_inotify,directorio_config, IN_MODIFY);
	free(directorio_config);
}

