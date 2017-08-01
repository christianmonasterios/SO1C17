#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include "configuracion.h"
#include "consolaKernel.h"
#include "planificador.h"
#include "estructuras.h"
#include "socket.h"
#include "log.h"

extern t_list *list_ejecutando;
extern t_list *list_finalizados;
extern t_list *list_bloqueados;
extern t_list *global_fd;
extern t_queue *cola_nuevos;
extern t_queue *cola_listos;
extern pthread_mutex_t mutex_lista_fs;
extern pthread_mutex_t mutex_lista_ejecutando;
extern pthread_mutex_t mutex_lista_finalizados;
extern pthread_mutex_t mutex_lista_bloqueados;
extern pthread_mutex_t mutex_cola_nuevos;
extern pthread_mutex_t mutex_cola_listos;
extern pthread_mutex_t mutex_actualizar_multip;
extern pthread_mutex_t mutex_planificar;
extern t_configuracion *config;
extern int flag_planificador;
//A partir de aca son cosas de mas para monitoreo
extern t_list *list_cpus;
extern t_list *list_consolas;
extern int diferencia_multi;

void imprimir_archivos_proceso(t_program *pp);

void leer_consola()
{
	while (1)
	{
		imprimir_menu();

		char *input, *input2;
		scanf("%ms", &input);

		switch (atoi(input))
		{
			case 1 :
				printf("Seleccione la lista o cola que desea visualizar:\n");
				printf("	1 - Todos\n");
				printf("	2 - Nuevos\n");
				printf("	3 - Listos\n");
				printf("	4 - Ejecutando\n");
				printf("	5 - Bloqueados\n");
				printf("	6 - Finalizados\n");

				scanf("%ms", &input2);

				if((atoi(input2) > 0) && (atoi(input2) < 7))
				{
					int number = atoi(input2);
					generar_listados(number);
				}
				else
					printf("No se ingreso un numero valido\n");

				free(input);
				free(input2);
				break;
			case 2 :
				printf("Indique el PID del proceso a consultar: ");
				scanf("%ms", &input2);
				int number = atoi(input2);

				if(existe_pid(number) == 1)
				{
					obtener_informacion(number);
				}
				else
					printf("El proceso buscado no existe\n");

				free(input);
				free(input2);
				break;
			case 3 :
				imprimir_tabla_archivos();
				free(input);
				break;
			case 4 :
				printf("El grado actual de multiprogramacion es: %i\n", config->grado_multiprog);
				printf("Indique el nuevo grado de multiprogramacion: ");
				scanf("%ms", &input2);
				int grado = atoi(input2);

				diferencia_multi = grado - config->grado_multiprog;
				config->grado_multiprog = grado;
				pthread_mutex_unlock(&mutex_actualizar_multip);

				free(input);
				free(input2);
				break;
			case 5 : ;
				printf("Indique el PID del proceso a finalizar: ");
				scanf("%ms", &input2);
				int numberKill = atoi(input2);

				if(existe_pid(numberKill) == 1)
				{
					forzar_finalizacion(numberKill, 0, 10, 1);
					printf("El proceso ha sido eliminado\n");
				}
				else
					printf("El proceso buscado no existe\n");

				free(input);
				free(input2);
				break;
			case 6 : ;
				if(flag_planificador==0)
				{
					flag_planificador = 1;
					pthread_mutex_lock(&mutex_planificar);
					printf("La planificacion se ha detenido\n");
				}
				else
					printf("La planificacion YA se encuentra detenida\n");
				free(input);
				break;
			case 7 : ;
				if(flag_planificador==1)
				{
					flag_planificador = 0;
					pthread_mutex_unlock(&mutex_planificar);
					printf("La planificacion se ha reanudado\n");
				}
				else
					printf("La planificacion YA se encuentra activa\n");
				free(input);
				break;
			case 8 : ;
				system("clear");
				break;
			case 9 :
				imprimir_info();
				break;
			default :
				printf("No se reconocio la opcion ingresada\n");
				free(input);
				break;
		}
	printf("\n\n");
	}
}

void generar_listados(int lista)
{
	if((lista == 1)	|| (lista == 2))
	{
		pthread_mutex_lock(&mutex_cola_nuevos);
		mostrar_cola(cola_nuevos, "Los siguientes son los procesos en la cola de Nuevos:\n");
		pthread_mutex_unlock(&mutex_cola_nuevos);
	}

	if((lista == 1)	|| (lista == 3))
	{
		pthread_mutex_lock(&mutex_cola_listos);
		mostrar_cola_listos(cola_listos, "Los siguientes son los procesos en la cola de Listos:\n");
		pthread_mutex_unlock(&mutex_cola_listos);
	}

	if((lista == 1)	|| (lista == 4))
	{
		pthread_mutex_lock(&mutex_lista_ejecutando);
		mostrar_listas(list_ejecutando, "Los siguientes son los procesos en ejecucion:\n");
		pthread_mutex_unlock(&mutex_lista_ejecutando);
	}

	if((lista == 1)	|| (lista == 5))
	{
		pthread_mutex_lock(&mutex_lista_bloqueados);
		mostrar_listas(list_bloqueados, "Los procesos que se encuentran bloqueados son:\n");
		pthread_mutex_unlock(&mutex_lista_bloqueados);
	}

	if((lista == 1)	|| (lista == 6))
	{
		pthread_mutex_lock(&mutex_lista_finalizados);
		mostrar_listas(list_finalizados, "Los siguientes son los procesos que ya han finalizado:\n");
		pthread_mutex_unlock(&mutex_lista_finalizados);
	}
}

void mostrar_cola(t_queue *cola, char *procesos)
{
	printf("###########################################\n");
	printf("%s", procesos);

	int i;
	int size = queue_size(cola);

	for(i=0;i<size;i++)
	{
		t_nuevo *pr = queue_pop(cola);
		printf("%d \n", pr->pid);
		queue_push(cola,pr);
	}
}

void mostrar_cola_listos(t_queue *cola, char *procesos)
{
	printf("###########################################\n");
	printf("%s", procesos);

	int i;
	int size = queue_size(cola);

	for(i=0;i<size;i++)
	{
		t_program *pr = queue_pop(cola);
		printf("%d \n", pr->PID);
		queue_push(cola,pr);
	}
}

void mostrar_listas(t_list *lista, char *procesos)
{
	printf("###########################################\n");
	printf("%s", procesos);

	void _mostrar(t_program *pr)
	{
		printf("%d \n", pr->PID);
	}

	list_iterate(lista, (void*)_mostrar);
}

void obtener_informacion(int pid)
{
	char *lista;
	int i, size;

	void _buscar_program(t_program *pr)
	{
		if(pr->PID == pid)
		{
			char *desc = devolver_descripcion_error(pr->pcb->exit_code);
			printf("Id Proceso: %i", pr->PID);
			printf("	Id Consola: %i\n", pr->CID);
			printf("Status de proceso: %s\n", lista);
			printf("Cantidad de allocations: %i\n", pr->allocs);
			printf("Cantidad de frees: %i\n", pr->frees);
			printf("Cantidad de Syscalls: %i\n", pr->syscall);
			printf("Cantidad de paginas: %i\n", pr->pcb->cant_pag);
			imprimir_archivos_proceso(pr);
			printf("Memory leaks: %i\n", pr->allocs_size - pr->frees_size);
			printf("Exit code: %i  Descripcion: %s\n", pr->pcb->exit_code, desc);
			free(desc);
		}
	}

	void _buscar_nuevo(t_nuevo *newnew)
	{
		if(newnew->pid == pid)
		{
			printf("Id Proceso: %i\n", newnew->pid);
			printf("Id Consola: %i\n", newnew->consola);
			printf("Status de proceso: %s\n", lista);
		}
	}

	lista =	strdup("Ejecutando");
	pthread_mutex_lock(&mutex_lista_ejecutando);
	list_iterate(list_ejecutando, (void*)_buscar_program);
	pthread_mutex_unlock(&mutex_lista_ejecutando);
	free(lista);

	lista =	strdup("Bloqueado");
	pthread_mutex_lock(&mutex_lista_bloqueados);
	list_iterate(list_bloqueados, (void*)_buscar_program);
	pthread_mutex_unlock(&mutex_lista_bloqueados);
	free(lista);

	lista =	strdup("Finalizado");
	pthread_mutex_lock(&mutex_lista_finalizados);
	list_iterate(list_finalizados, (void*)_buscar_program);
	pthread_mutex_unlock(&mutex_lista_finalizados);
	free(lista);

	lista =	strdup("Cola de nuevos");
	pthread_mutex_lock(&mutex_cola_nuevos);
	size = queue_size(cola_nuevos);
	for(i=0;i<size;i++)
	{
		t_program *pr = queue_pop(cola_nuevos);
		_buscar_program(pr);
		queue_push(cola_nuevos,pr);
	}
	size = 0; i = 0;
	pthread_mutex_unlock(&mutex_cola_nuevos);
	free(lista);

	lista =	strdup("Cola de listos");
	pthread_mutex_lock(&mutex_cola_listos);
	size = queue_size(cola_listos);
	for(i=0;i<size;i++)
	{
		t_program *pr = queue_pop(cola_listos);
		_buscar_program(pr);
		queue_push(cola_listos,pr);
	}
	size = 0; i = 0;
	pthread_mutex_unlock(&mutex_cola_listos);
	free(lista);
}

void imprimir_archivos_proceso(t_program *pp)
{
	printf("Tabla de archivos de proceso:\n");
	void _imprimir(t_TAP *tap)
	{
		printf("	File descriptor: %d\n", tap->FD);
		printf("	File descriptor Global: %d\n", tap->GFD);
		printf("	Flags asignados: %s\n", tap->flag);
	}

	list_iterate(pp->TAP, (void*)_imprimir);
}

void imprimir_tabla_archivos()
{
	printf("Tabla de archivos:\n");
	void _imprimir(t_TAG *tg)
	{
		printf("Archivo: %s\n", tg->path);
		printf("File Descriptor: %d\n", tg->FD);
		printf("Open: %i\n\n", tg->open_);
	}

	pthread_mutex_lock(&mutex_lista_fs);
	list_iterate(global_fd, (void*)_imprimir);
	pthread_mutex_unlock(&mutex_lista_fs);
}

void imprimir_menu()
{
	printf("Seleccione el numero de la opcion a ejecutar\n");
	printf("	1 - Listar procesos\n");
	printf("	2 - Obtener informacion de proceso\n");
	printf("	3 - Obtener tabla global de archivos\n");
	printf("	4 - Modificar grado de multiprogramacion\n");
	printf("	5 - Finalizar proceso\n");
	printf("	6 - Detener planificacion\n");
	printf("	7 - Reanudar planificacion\n");
	printf("	8 - Limpiar pantalla\n");
	printf("	9 - Imprimir info de sistema\n\n");
}

int existe_pid(int pid)
{
	int size, i;
	int encontrado = 0;

	void _buscar_program(t_program *pr)
	{
		if(pr->PID == pid)
			encontrado = 1;
	}

	void _buscar_program_nuevo(t_nuevo *nv)
	{
		if(nv->pid == pid)
			encontrado = 1;
	}

	pthread_mutex_lock(&mutex_lista_ejecutando);
	list_iterate(list_ejecutando, (void*)_buscar_program);
	pthread_mutex_unlock(&mutex_lista_ejecutando);

	pthread_mutex_lock(&mutex_lista_bloqueados);
	list_iterate(list_bloqueados, (void*)_buscar_program);
	pthread_mutex_unlock(&mutex_lista_bloqueados);

	pthread_mutex_lock(&mutex_lista_finalizados);
	list_iterate(list_finalizados, (void*)_buscar_program);
	pthread_mutex_unlock(&mutex_lista_finalizados);

	pthread_mutex_lock(&mutex_cola_nuevos);
	size = queue_size(cola_nuevos);
	for(i=0;i<size;i++)
	{
		t_nuevo *pr = queue_pop(cola_nuevos);
		if(pr != NULL)
		{
			_buscar_program_nuevo(pr);
			queue_push(cola_nuevos,pr);
		}
	}
	size = 0; i = 0;
	pthread_mutex_unlock(&mutex_cola_nuevos);

	pthread_mutex_lock(&mutex_cola_listos);
	size = queue_size(cola_listos);
	for(i=0;i<size;i++)
	{
		t_program *pr = queue_pop(cola_listos);
		if (pr != NULL)
		{
		_buscar_program(pr);
		queue_push(cola_listos,pr);
		}
	}
	size = 0; i = 0;
	pthread_mutex_unlock(&mutex_cola_listos);

	return encontrado;
}

void imprimir_info()
{
	printf("Procesos en Nuevos: %d\n", queue_size(cola_nuevos));
	printf("Procesos en Listos: %d\n", queue_size(cola_listos));
	printf("Procesos en Ejecutando: %d\n", list_size(list_ejecutando));
	printf("Procesos en Bloqueados: %d\n", list_size(list_bloqueados));
	printf("Procesos en Finalizados: %d\n", list_size(list_finalizados));

	void _imprimir_cpu(t_cpu *cpu)
	{
		printf("Numero de CPU: %d\n", cpu->cpu_id);
		printf("Estado de ejecutando: %d\n", cpu->ejecutando);
		if(cpu->program==NULL)
			printf("La cpu no tiene procesos asignados\n");
		else
			printf("La cpu tiene asignado el proceso: %d\n", cpu->program->PID);
	}

	void _imprimir_consola(t_consola *consola)
	{
		printf("Numero de Consola: %d\n", consola->CID);
	}

	printf("\nLista de CPU's\n");
	list_iterate(list_cpus, (void*)_imprimir_cpu);

	printf("\nLista de Consolas\n");
	list_iterate(list_consolas, (void*)_imprimir_consola);
}

char *devolver_descripcion_error(int codigo)
{
	char *descripcion;

	switch (codigo)
	{
	case 0 :
		descripcion = strdup("El programa finalizo correctamente");
		break;
	case -1 :
		descripcion = strdup("No se pudieron reservar recursos para ejecutar el programa");
		break;
	case -2 :
		descripcion = strdup("El programa intento acceder a un archivo que no existe");
		break;
	case -3 :
		descripcion = strdup("El programa intento leer un archivo sin permisos");
		break;
	case -4 :
		descripcion = strdup("El programa intento escribir un archivo sin permisos");
		break;
	case -5 :
		descripcion = strdup("Excepcion de memoria");
		break;
	case -6 :
		descripcion = strdup("Finalizado a traves de desconexion de consola");
		break;
	case -7 :
		descripcion = strdup("Finalizado a traves del comando Finalizar Programa de la consola");
		break;
	case -8 :
		descripcion = strdup("Se intento reservar mas memoria que el tamanio de una pagina");
		break;
	case -9 :
		descripcion = strdup("No se pueden asignar mas paginas al proceso");
		break;
	case -11 :
		descripcion = strdup("Se intento acceder a un semaforo inexistente");
		break;
	case -12 :
		descripcion = strdup("Se intento acceder a una variable inexistente");
		break;
	case -13 :
		descripcion = strdup("No hay espacio suficiente para almacenar en archivo");
		break;
	case -15 :
		descripcion = strdup("Se intento borrar un archivo inexistente");
		break;
	case -16 :
		descripcion = strdup("Se intento cerrar un archivo inexistente");
		break;
	case -17 :
		descripcion = strdup("Se intento mover un puntero en un archivo no abierto");
		break;
	case -18 :
		descripcion = strdup("Se intento escribir un archivo no abierto");
		break;
	case -20 :
		descripcion = strdup("Error sin definicion");
		break;
	case 1 :
		descripcion = strdup("El proceso aun no ha finalizado");
		break;
	default :
		descripcion = strdup("Sin descripcion");
		break;
	}

	return descripcion;
}
