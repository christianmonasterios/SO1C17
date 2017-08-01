#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <commons/string.h>
#include <commons/log.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "log.h"
#include "manejo_errores.h"

extern pthread_mutex_t mutex_socket;

int iniciar_socket_cliente(char *ip, int puerto_conexion, int *control) {
	int connected_socket;
	struct sockaddr_in dest;
	*control = 0;

	//Creating socket
	if ((connected_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		*control = 1;
		error_sockets(control, "");
	} else
		escribir_log("Kernel - Socket creado");

	dest.sin_family = AF_INET;
	dest.sin_port = htons(puerto_conexion);
	dest.sin_addr.s_addr = inet_addr(ip);

	//Connecting socket
	if (connect(connected_socket, (struct sockaddr*) &dest, sizeof(dest))
			!= 0) {
		*control = 2;
		error_sockets(control, "");
	}
	return connected_socket;
}

int iniciar_socket_server(char *ip, int puerto_conexion, int *controlador) {
	int socketServidor;
	struct sockaddr_in my_addr;
	int yes = 1;
	int BACKLOG = 20; //Cantidad de conexiones maximas
	controlador = 0;

	//Creating socket
	if ((socketServidor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		*controlador = 3;
		error_sockets(controlador, "");
	}
	//printf("created socket\n");

	setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(puerto_conexion);
	my_addr.sin_addr.s_addr = inet_addr(ip);

	//Binding socket
	if (bind(socketServidor, (struct sockaddr*) &my_addr,
			sizeof(struct sockaddr_in)) != 0) {
		*controlador = 4;
		error_sockets(controlador, "");
	} else {
		escribir_log("Kernel - Socket server creado");
	}

	//Listening socket
	if (listen(socketServidor, BACKLOG) != 0) {
		*controlador = 5;
		error_sockets(controlador, "");
	}

	return socketServidor;
}

int escuchar_conexiones(int socketServidor, int *controlador) {
	int client_sock_accepted;
	int c;
	struct sockaddr_in client;
	int BACKLOG = 20; //Cantidad de conexiones maximas
	controlador = 0;

	//Listening socket
	if (listen(socketServidor, BACKLOG) != 0) {
		*controlador = 5;
		error_sockets(controlador, "");
	}
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	client_sock_accepted = accept(socketServidor, (struct sockaddr *) &client,
			(socklen_t*) &c);
	if (client_sock_accepted < 0) {
		*controlador = 6;
		error_sockets(controlador, "");
	} else
		escribir_log_con_numero(
				"Kernel - Nueva conexion aceptada para socket: ",
				client_sock_accepted);

	return client_sock_accepted;
}

int aceptar_conexion(int socketServidor, int *controlador) {
	int c = 0;
	struct sockaddr_in client;
	*controlador = 0;

	int client_sock_accepted = accept(socketServidor,
			(struct sockaddr *) &client, (socklen_t*) &c);
	if (client_sock_accepted < 0) {
		*controlador = 6;
		error_sockets(controlador, "");
	} else
		escribir_log_con_numero(
				"Kernel - Nueva conexion aceptada para socket: ",
				client_sock_accepted);

	return client_sock_accepted;
}

int enviar(int socket_emisor, char *mensaje_a_enviar, int *controlador)
{
	escribir_log_compuesto("MENSAJE_ENVIAR: ", mensaje_a_enviar);

	int ret;
	signal(SIGPIPE, SIG_IGN);
	size_t sbuffer = sizeof(char) * string_length(mensaje_a_enviar);
	*controlador = 0;

	char *buffer = string_substring(mensaje_a_enviar, 0, sbuffer);

	if ((ret = send(socket_emisor, buffer, sbuffer, MSG_NOSIGNAL)) < 0) {
		*controlador = 7;
		char *emisor = string_itoa(socket_emisor);
		error_sockets(controlador, emisor);
		free(emisor);
	}

	free(buffer);
	return ret;
}

char *recibir(int socket_receptor, int *controlador)
{
	pthread_mutex_lock(&mutex_socket);

	int ret;
	char *buffer = malloc(13 + 1);
	memset(buffer,'\0',14);
	*controlador = 0;

	if((ret = recv(socket_receptor, buffer, 13, MSG_WAITALL)) <= 0)
	{
		if(ret == 0)
		{
			*controlador = 8;
			char *receptor = string_itoa(socket_receptor);
			error_sockets(controlador, receptor);
			free(receptor);
		}
		else
		{
			*controlador = 1;
			error_sockets(controlador, "");
		}
	}

	//if((strncmp(cod,"P13",3)==0)||(strncmp(cod,"P12",3)==0)||(strncmp(cod,"P20",3)==0))
	//{
		char *str_size = string_substring(buffer, 3, 10);
		//memcpy(str_size,buffer+3,10);

		int size = atoi(str_size);
		free(str_size);
		char *resto_mensaje = malloc(size);
		if(size>0) recv(socket_receptor, resto_mensaje, size, 0);

		char *buffer_aux= malloc(size+14);
		memcpy(buffer_aux,buffer,13);
		memcpy(buffer_aux+13,resto_mensaje,size);
		buffer_aux[size+13] = '\0';

		free(resto_mensaje);

		if(strncmp(buffer,"P",1)==0){
			escribir_log_compuesto("MENSAJE_RECIBIDO: ", buffer);
		}else{
			escribir_log_compuesto("MENSAJE_RECIBIDO: ", buffer_aux);
		}

	//}
	//else
	//{
	//	buffer_aux[ret] = '\0';
	//	buffer_aux = strdup(buffer);
	//	escribir_log_compuesto("recibido: ", buffer);
	//}
	free(buffer);

	pthread_mutex_unlock(&mutex_socket);

	return buffer_aux;
}

void cerrar_conexion(int socket_)
{
	close(socket_);
}

int enviar_pcb(int socket_emisor, char *mensaje_a_enviar, int *controlador,int size)
{
	int ret;
	signal(SIGPIPE, SIG_IGN);
	*controlador = 0;

	if ((ret = send(socket_emisor, mensaje_a_enviar, size, MSG_NOSIGNAL)) < 0) {
		//close(socket_emisor);
		*controlador = 7;

	} else {
		//Este mensaje debera esta en la funcion que invoque esta
		//escribir_log_con_numero("Kernel - Exito al enviar mensaje a PID: ", *prog->PID);
	}

	return ret;
}
