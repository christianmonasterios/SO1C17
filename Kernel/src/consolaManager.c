#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include "consolaManager.h"
#include "mensaje.h"
#include "planificador.h"
#include "estructuras.h"
#include "cpuManager.h"
#include "manejo_errores.h"
#include "socket.h"
#include "log.h"

extern t_list *list_consolas;
extern t_queue *cola_nuevos;
extern pthread_mutex_t mutex_lista_consolas;
extern pthread_mutex_t mutex_cola_nuevos;
extern sem_t sem_nuevos;
extern int ultimo_pid;
fd_set master;
fd_set read_fds;
int fdmax;
int controlador = 0;


void realizar_handShake_consola(int nuevo_socket)
{
	//Envio mensaje a consola pidiendo sus datos
	char *var = strdup("");
	char *mensaje = armar_mensaje("K00",var);
	enviar(nuevo_socket, mensaje, &controlador);
	escribir_log("Enviado mensaje para handshake con consola");//despues borrar
	free(var);

	if (controlador > 0)
	{
		cerrar_conexion(nuevo_socket);
		escribir_error_log("Fallo el Handshake con el administrador consola");
		FD_CLR(nuevo_socket, &master);
	}
	else
	{
		char *respuesta = recibir(nuevo_socket, &controlador);
		escribir_log("Recibido mensaje para handshake con consola");//despues borrar

		if (controlador > 0)
		{
			cerrar_conexion(nuevo_socket);
			escribir_error_log("Fallo el Handshake con el administrador consola");
			FD_CLR(nuevo_socket, &master);
		}
		else
		{
			//Aca deberia ir la validacion si el mensaje corresponde a una consola
			char *header = get_header(respuesta);
			if(comparar_header("C", header))
			{
				escribir_log("Se ha conectado una nueva consola");
				//Es una Consola, se puede agregar
				t_consola *nueva_consola = malloc(sizeof(t_consola));
				nueva_consola->socket = nuevo_socket;
				nueva_consola->CID = get_CID();

				pthread_mutex_lock(&mutex_lista_consolas);
				list_add(list_consolas, nueva_consola);
				pthread_mutex_unlock(&mutex_lista_consolas);
			}
			else
			{
				//El recien conectado NO corresponde a una consola
				escribir_log("El administrador de consola rechazo una conexion");
				char *mensaje = "Perdon no sos una Consola, Chau!";
				enviar(nuevo_socket, mensaje, &controlador);
				cerrar_conexion(nuevo_socket);
				FD_CLR(nuevo_socket, &master);
			}
			free(mensaje);
			free(header);
		}
	free(respuesta);
	}
}

int get_CID()
{
	int ultimo_id = 1;

	void _mayor(t_consola *consola)
	{
		if(consola->CID == ultimo_id)
		{
			ultimo_id++;
		}
	}
	list_iterate(list_consolas, (void*)_mayor);
	return ultimo_id;
}

void responder_solicitud_consola(int socket, char *mensaje)
{
	char *codigo = get_codigo(mensaje);
	int cod = atoi(codigo);

	switch(cod)
	{
		case 1 :
			escribir_log("Llego una solicitud de inicio de programa");
			responder_peticion_prog(socket, mensaje);
			break;
		case 2 : ;
			escribir_log("Llego una solicitud de finalizacion de programa");
			char *pid_c = get_mensaje(mensaje);
			int pid = atoi(pid_c);
			forzar_finalizacion(pid, 0, 7, 1);
			free(pid_c);
			break;
		case 3 : ;
			//char *con = get_mensaje(mensaje);
			//int consola_id = atoi(con);
			//forzar_finalizacion(0, consola_id, 6, 0);
			desconectar_consola(socket);
			//free(con);
			break;
		case 4 : ;
			escribir_log("Llego una solicitud de finalizacion de programa por desconexion de consola");
			char *pid_cc = get_mensaje(mensaje);
			int pidd = atoi(pid_cc);
			forzar_finalizacion(pidd, 0, 6, 1);
			free(pid_cc);
			break;
		default : ;
			escribir_log("El administrador de consola recibio un mensaje desconocido");
			//No se comprende el mensaje recibido por consola
			char *msj_unknow = "K08";
			enviar(socket, msj_unknow, &controlador);
			if (controlador > 0)
			{
				desconectar_consola(socket);
				int id_consola2 = buscar_consola(socket);
				forzar_finalizacion(0, id_consola2, 20, 0);
			}
	}
	free(codigo);
}

void eliminar_consola(int consola_id)
{
	bool _localizar(t_consola *con)
	{
		return (con->CID == consola_id);
	}

	void liberar_consola(t_consola *consola)
	{
		free(consola);
	}

	pthread_mutex_lock(&mutex_lista_consolas);
	list_remove_and_destroy_by_condition(list_consolas, (void*)_localizar, (void*)liberar_consola);
	pthread_mutex_unlock(&mutex_lista_consolas);
}

void responder_peticion_prog(int socket, char *mensaje)
{
	char *codigo_new = get_mensaje(mensaje);
	char *ult_pid = string_itoa(ultimo_pid);

	char *mensaje_conf =  armar_mensaje("K26", ult_pid);
	enviar(socket, mensaje_conf, &controlador);

	if(controlador > 0) desconectar_consola(socket);
	else
	{
		t_nuevo *nuevo_proc = malloc(sizeof(t_nuevo));
		nuevo_proc->pid = ultimo_pid;
		nuevo_proc->codigo = codigo_new;
		nuevo_proc->new_socket = socket;
		nuevo_proc->consola = buscar_consola(socket);

		escribir_log("Se ha movido un proceso a cola de nuevos");

		pthread_mutex_lock(&mutex_cola_nuevos);
		queue_push(cola_nuevos, nuevo_proc);
		pthread_mutex_unlock(&mutex_cola_nuevos);

		ultimo_pid ++;
		sem_post(&sem_nuevos);
	}
	free(mensaje_conf);
	free(ult_pid);
}

int buscar_consola(int socket)
{
	bool _buscar_consola_lst(t_consola *consola)
	{
		return (consola->socket == socket);
	}

	pthread_mutex_lock(&mutex_lista_consolas);
	t_consola *cons = list_find(list_consolas, (void*)_buscar_consola_lst);
	pthread_mutex_unlock(&mutex_lista_consolas);

	if(cons == NULL)
		return 0;
	else
		return cons->CID;
}

void desconectar_consola(int socket)
{
	int consola_muere = buscar_consola(socket);
	if(consola_muere)
	{
		eliminar_consola(consola_muere);
	}
	cerrar_conexion(socket);
	FD_CLR(socket, &master);
}
