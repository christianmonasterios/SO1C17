#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
#include <commons/string.h>
#include "socket_client.h"
#include "hilo_usuario.h"
#include "hilo_programa.h"
#include "estructuras.h"
#include "log.h"
#include "mensaje.h"
#include <signal.h>

extern t_dictionary * p_pid;
extern t_dictionary * h_pid;
extern t_dictionary * sem;
extern t_dictionary * impresiones;
extern t_dictionary * tiempo;
extern t_dictionary *no_iniciados;
extern sem_t semaforo;
extern sem_t x;
extern int flag;
extern int socket_;
pthread_t hiloPrograma;
extern char *aImprimir;

void escuchar_mensaje();
void verificacion_finalizar(char * pid);
void finalizar(char *pid, int socket_);
void finalizar_no_iniciados(char * pid, int socket_);
void senial();

void escuchar_mensaje()
{
	signal(SIGKILL,senial);
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	char *mensaje;
	char *mensaje2;
	sem_init(&semaforo,0,2);
	sem_init(&x,0,2);

	while(flag==0)
	{
		sem_wait(&x);
		sem_wait(&semaforo);
		t_chequeo *sema = malloc(sizeof(t_chequeo));
		t_chequeo *smod = malloc(sizeof(t_chequeo));
		t_impresiones *cant = malloc(sizeof(t_impresiones));
		t_hilo *hilo = malloc(sizeof(t_hilo));
		mensaje = recibir(socket_,13);
		mensaje2 = get_codigo(mensaje);

		switch(atoi(mensaje2))
		{
			case 4: ;
				char *pid;
				//char * elemento2;

				sema->valor=0;
				sem_init(&sema->val,0,0);
				//sem_wait(&sema->val);
				cant->cantidad=0;

				pid = recibir(socket_,1);

				dictionary_put(sem,pid,sema);
				dictionary_put(impresiones,pid,cant);

				pthread_create(&hiloPrograma,NULL,(void*)programa,pid);
				hilo->hilo= hiloPrograma;

				escribir_log_con_numero("Se inicio el programa: ", atoi(pid));
				printf("Se inicio el programa: %s\n", pid);

				dictionary_put(p_pid,pid,hilo);
				dictionary_put(h_pid,string_itoa(hiloPrograma),pid);

				/*bool encontrar(void *pid_)
				{
					return !strcmp(pid,pid_);
				}

				elemento2 = list_remove_by_condition(no_iniciados, encontrar);*/
				dictionary_remove(no_iniciados,pid);

				//free(elemento2);

				break;
			case 5: ;
				char *pid3;
				escribir_log("No se pudo iniciar el programa por falta de memoria");
				printf("No se pudo iniciar el programa por falta de memoria\n");

				pid3 = recibir(socket_,1);
				finalizar_no_iniciados(pid3, socket_);


				free(sema);
				free(pid3);
				free(cant);
				free(smod);
				free(hilo);
				break;
			case 26:;
				char *pid7 = recibir(socket_,1);

				escribir_log_compuesto("Se le asigno el pid ", pid7);
				printf("Se le asigno el pid %s\n ", pid7);

				//list_add(no_iniciados,pid7);
				dictionary_put(no_iniciados, pid7,"");

				//free(pid7);
				break;
			case 9: ;
				char *pid2 = recibir(socket_,1);
				char *size = get_payload(mensaje);
				char *msj = recibir(socket_, atoi(size)-1);

				aImprimir = strdup("Mensaje de PID ");
				string_append(&aImprimir,pid2);
				string_append(&aImprimir," a imprimir: ");
				string_append(&aImprimir,msj);

				smod=dictionary_get(sem,pid2);
				smod->valor=1;
				sem_post(&smod->val);
				sem_wait(&x);

				free(sema);
				free(cant);
				free(hilo);
				free(pid2);
				break;
			case 10: ;
				char *pid33;

				pid33 = recibir(socket_,1);

				verificacion_finalizar(pid33);

				free(sema);
				free(cant);
				free(smod);
				free(hilo);
				free(pid33);
				break;
			default:
				escribir_log("Mensaje incorrecto del Kernel");
				flag=1;
				free(sema);
				free(cant);
				free(smod);
				free(hilo);
				break;
		}
		sem_post(&x);
		sem_post(&semaforo);
		free(mensaje);
		free(mensaje2);
	}

	desconectar_consola();
	pthread_exit(NULL);
}

void verificacion_finalizar(char * pid)
{
	/*bool encontrar(void *pid_)
	{
		return !strcmp(pid,pid_);
	}*/

	if (dictionary_has_key(no_iniciados,pid)){
		finalizar_no_iniciados(pid,socket_);
	}
	else finalizar(pid,socket_);
}



void finalizar(char *pid, int socket_)
{
	if(dictionary_has_key(p_pid,pid))
	{
		//char *pid2;
		t_hilo *hilo = dictionary_get(p_pid,pid);
		char *var = string_itoa(hilo->hilo);

		if(pthread_cancel(hilo->hilo)==0)
		{
			//pid2 = dictionary_get(h_pid,var);
			escribir_log_con_numero("Se finalizo el programa: ", atoi(pid));
			printf("Se finalizo el programa: %d\n", atoi(pid));
			tiempofinal_impresiones(pid);

			free(dictionary_remove(h_pid,var));
			free(dictionary_remove(p_pid,pid));
			free(dictionary_remove(impresiones,pid));
			free(dictionary_remove(sem,pid));
			free(dictionary_remove(tiempo,pid));
		}
		else
		{
			escribir_log("No se pudo finalizar el programa");
			printf("No se pudo finalizar el programa");
		}
	}
}

void finalizar_no_iniciados(char * pid,int socket_)
{
	/*char * elemento;

	bool remover(void *pid_)
	{
		return !strcmp(pid,pid_);
	}

	elemento = list_remove_by_condition(no_iniciados, remover);*/

	escribir_log_con_numero("Se finalizo el programa: ", atoi(pid));

	dictionary_remove(no_iniciados, pid);

	printf("\nInicio de ejecucion: 0\n");
	printf("Fin de ejecucion: 0\n");
	printf("Cantidad de impresiones: 0\n");
	printf("Tiempo total de ejecucion en segundos: 0\n\n");

	//free(elemento);
}

void senial()
{
	pthread_exit(NULL);
}
