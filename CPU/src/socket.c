#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "estructuras.h"
#include <commons/string.h>
#include "log.h"
#include "socket.h"
#include <commons/log.h>

int iniciar_socket_cliente(char *ip, int puerto_conexion, int *control)
{
	int connected_socket;
	struct sockaddr_in dest;
	*control = 0;

	//Creating socket
	if ((connected_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		*control = 1;

	}else escribir_log("Socket creado",1);

	dest.sin_family = AF_INET;
	dest.sin_port = htons( puerto_conexion );
	dest.sin_addr.s_addr = inet_addr( ip );

	//Connecting socket
	if (connect(connected_socket, (struct sockaddr*) &dest, sizeof(dest)) != 0)
	{
		*control = 2;

	}else
	{

	}
	return connected_socket;
}

int enviar(int socket_emisor,void * mensaje_a_enviar, int *controlador,int size)
{

	int ret;
	//signal(SIGPIPE, SIG_IGN);
	*controlador = 0;

	if ((ret = send(socket_emisor, mensaje_a_enviar,size, 0)) <= 0)
	{
		//close(socket_emisor);
		*controlador = 7;

	} else
	{
		//Este mensaje debera esta en la funcion que invoque esta
		//escribir_log_con_numero("Kernel - Exito al enviar mensaje a PID: ", *prog->PID);
	}
	return ret;
}

void recibir(int socket_receptor, int *controlador,void *buff,int size)
{
	int ret;

	*controlador = 0;

	if ((ret = recv(socket_receptor, buff, size,MSG_WAITALL)) <= 0)
	{
		//printf("error receiving or connection lost \n");
		if (ret == 0)
		{
			*controlador = 8;
		} else {
			//printf("error recibiendo el mensaje \n");
				}
		*controlador = 1;
		//close(socket_receptor);
	}
}

void cerrar_conexion(int socket_)
{
	close(socket_);
}
