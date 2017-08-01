#include <commons/string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log.h"

int iniciar_socket_cliente(char *ip, int puerto/*char *puerto*/)
{
	int connected_socket, puerto_conexion;
	struct sockaddr_in dest;

	//Creating socket
	if ((connected_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		escribir_error_log("Error creando socket");
	}
	escribir_log("Socket cliente creado");

	puerto_conexion = puerto;

	dest.sin_family = AF_INET;
	dest.sin_port = htons( puerto_conexion );
	dest.sin_addr.s_addr = inet_addr( ip );

	//Connecting socket
	if (connect(connected_socket, (struct sockaddr*) &dest, sizeof(dest)) != 0)
		escribir_error_log("Error conectando socket a Server");
	else
		escribir_log_compuesto("Conectado a servidor: ", ip);

	return connected_socket;
}

int enviar(int socket_emisor, char *mensaje_a_enviar, int tamanio)
{
	int ret;

	size_t sbuffer = sizeof(char)* tamanio;

	char *buffer = string_substring(mensaje_a_enviar,0,sbuffer);

	if ((ret = send(socket_emisor, buffer, sbuffer, 0)) < 0) {
		escribir_error_log("Error en el envio del mensaje");
	}
	free(buffer);
	return ret;
}

char *recibir(int socket_receptor,int tamanio)
{
	int ret;

	char buffer2[tamanio];
	char *buffer;

	if ((ret = recv(socket_receptor,(void *) buffer2, tamanio, 0)) <= 0)
	{
		if (ret == 0)
			escribir_error_log("El socket server se desconecto");
		else
			escribir_error_log("Error recibiendo el mensaje de Server");
		//close(socket_receptor);
	}

	buffer2[ret]='\0';
	buffer = strdup(buffer2);
	return buffer;
}

void cerrar_conexion(int socket_)
{
	close(socket_);
}
