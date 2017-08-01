#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/string.h>
#include <commons/log.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "log.h"
#include "manejo_errores.h"

//extern t_log *log;

int iniciar_socket_cliente(char *ip, int puerto_conexion, int *control)
{
	int connected_socket;
	struct sockaddr_in dest;
	*control = 0;

	//Creating socket
	if ((connected_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		*control = 1;
		error_sockets(control, "");
	}
	else escribir_log("Memoria - Socket creado");

	dest.sin_family = AF_INET;
	dest.sin_port = htons( puerto_conexion );
	dest.sin_addr.s_addr = inet_addr( ip );

	//Connecting socket
	if (connect(connected_socket, (struct sockaddr*) &dest, sizeof(dest)) != 0)
	{
		*control = 2;
		error_sockets(control, "");
	}
	return connected_socket;
}

int iniciar_socket_server(char *ip, int puerto_conexion, int *controlador)
{
	int socketServidor;
	struct sockaddr_in my_addr;
	int yes = 1;
	int BACKLOG = 20; //Cantidad de conexiones maximas
	controlador = 0;

	//Creating socket
	if ((socketServidor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		*controlador = 3;
		error_sockets(controlador, "");
	}
	//printf("created socket\n");

	setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons( puerto_conexion );
	my_addr.sin_addr.s_addr = inet_addr( ip );

	//Binding socket
	if (bind(socketServidor, (struct sockaddr*) &my_addr, sizeof(struct sockaddr_in)) != 0)
	{
		*controlador = 4;
		error_sockets(controlador, "");
	}
	else
	{
		escribir_log("Se inicia Servidor de Memoria");
	}

	//Listening socket
	if (listen(socketServidor, BACKLOG) != 0)
	{
		*controlador = 5;
		error_sockets(controlador, "");
	}

	return socketServidor;
}

int escuchar_conexiones(int socketServidor, int *controlador)
{
	int client_sock_accepted;
	int c;
	struct sockaddr_in client;
	int BACKLOG = 20; //Cantidad de conexiones maximas
	controlador = 0;

	//Listening socket
	if (listen(socketServidor, BACKLOG) != 0)
	{
		*controlador = 5;
		error_sockets(controlador, "");
	}
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	client_sock_accepted = accept(socketServidor, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock_accepted < 0)
	{
		*controlador = 6;
		error_sockets(controlador, "");
	}
	else
	escribir_log_con_numero("Memoria - Nueva conexion aceptada para socket: ", client_sock_accepted);

	return client_sock_accepted;
}

int aceptar_conexion(int socketServidor, int *controlador)
{
	int c;
	struct sockaddr_in client;

	int client_sock_accepted = accept(socketServidor, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock_accepted < 0)
	{
		*controlador = 6;
		error_sockets(controlador, "");
	}
	else
	escribir_log_con_numero("Memoria - Nueva conexion aceptada para socket: ", client_sock_accepted);

	return client_sock_accepted;
}

int enviar(int socket_emisor, char *mensaje_a_enviar, int *controlador)
{
	int ret;
	signal(SIGPIPE, SIG_IGN);
	size_t sbuffer = sizeof(char)*string_length(mensaje_a_enviar);
	*controlador = 0;

	char *buffer = string_substring(mensaje_a_enviar,0,sbuffer);

	if ((ret = send(socket_emisor, buffer, sbuffer, MSG_NOSIGNAL)) < 0)
	{
		*controlador = 7;
		char *error= string_itoa(socket_emisor);
		error_sockets(controlador, error);
		free(error);
	}

	free(buffer);
	return ret;
}

char *recibir(int socket_receptor, int *controlador)
{
	int ret;

	char *buffer = malloc(1024);

	*controlador = 0;

	if ((ret = recv(socket_receptor, buffer, 1024, 0)) <= 0)
	{
		//printf("error receiving or connection lost \n");
		if (ret == 0)
		{
			*controlador = 8;
			char *error = string_itoa(socket_receptor);
			error_sockets(controlador,error);
			free(error);
		}
		*controlador = 1;
		error_sockets(controlador, "");
	}

	//char *buffer_aux= strdup(buffer);
	//free(buffer);
	return buffer;
}

void cerrar_conexion(int socket_)
{
	close(socket_);
}
