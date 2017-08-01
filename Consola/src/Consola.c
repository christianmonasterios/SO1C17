#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include "estructuras.h"
#include "socket_client.h"
#include "escuchar_mensaje.h"
#include "hilo_usuario.h"
#include "parametros.h"
#include "mensaje.h"
#include "log.h"
#include <semaphore.h>
#include <time.h>
#include <signal.h>

t_consola *arch_config;
pthread_t hiloUsuario;
pthread_t hiloMensaje;
t_log * log_;
t_dictionary *p_pid;
t_dictionary *h_pid;
t_dictionary *sem;
t_dictionary *tiempo;
t_dictionary *impresiones;
t_dictionary *procesos;
t_dictionary *no_iniciados;
sem_t semaforo;
int tamAimprimir;
int flag;
int socket_;
char *aImprimir;
sem_t x;

void leer_archivo_configuracion(char *ruta);
void handshake(int socket_);
void hilousuario();
void escuchar_mensaje();

int main(int argc, char * argv[])
{
	signal(SIGINT,desconectar_consola);
	crear_archivo_log("/home/utnso/log_consola.txt");
	escribir_log("Se inicializan los parametros y diccionarios");
	inicializar_parametros();
	escribir_log("Se lee el archivo de parametros");
	leer_archivo_configuracion(argv[1]);
	socket_ = iniciar_socket_cliente(arch_config->ip, arch_config->puerto);

	handshake(socket_);

	pthread_create(&hiloUsuario,NULL,(void*)hilousuario,NULL);
	pthread_create(&hiloMensaje,NULL,(void*)escuchar_mensaje,NULL);

	pthread_join(hiloUsuario,NULL);
	pthread_join(hiloMensaje,NULL);

	liberar_memoria();
	cerrar_conexion(socket_);

	return EXIT_SUCCESS;
}

void leer_archivo_configuracion(char * ruta)
{
	t_config *configConsola = config_create(ruta);
	string_append(&arch_config->ip, config_get_string_value(configConsola,"IP_KERNEL"));
	arch_config->puerto = config_get_int_value(configConsola,"PUERTO_KERNEL");
	config_destroy(configConsola);
}

void handshake(int socket_)
{
	char *mensaje;
	char *mensaje_recibido = recibir(socket_, 13);

	char *otro_aux = get_header(mensaje_recibido);

	if(!strcmp(otro_aux,"K"))
	{
		mensaje = armar_mensaje("C00","");
		enviar(socket_,mensaje,string_length(mensaje));
	}

	escribir_log("Se realizo el handshake");
	free(mensaje);
	free(mensaje_recibido);
	free(otro_aux);
}
