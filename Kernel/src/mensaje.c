#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include "mensaje.h"

char *armar_mensaje(char *identificador, char *mensaje)
{
	char *resultado = strdup(identificador);
	int length = string_length(mensaje);
	char *payload_char = string_itoa(length);
	int size_payload = string_length(payload_char);
	char *completar = string_repeat('0', 10 - size_payload);

	string_append(&resultado, completar);
	string_append(&resultado, payload_char);
	string_append(&resultado, mensaje);

	free(payload_char);
	free(completar);
	return resultado;
}

char *armar_mensaje_pcb(char *identificador, char *mensaje,int sizepcb)
{
	char *resultado = malloc(13+sizepcb);
	char *payload_char = string_itoa(sizepcb);
	int size_payload = string_length(payload_char);
	char *completar = string_repeat('0', 10 - size_payload);

	memcpy(resultado,identificador,3);
	memcpy(resultado+3,completar,10 - size_payload);
	memcpy(resultado+3+10-size_payload,payload_char,size_payload);
	memcpy(resultado+13,mensaje,sizepcb);

	free(payload_char);
	free(completar);
	return resultado;
}

//devuelve el header del mensaje
char *get_header(char *mensaje)
{
	return string_substring(mensaje, 0, 1);
}

int comparar_header(char *identificador, char *header)
{
	return !strcmp(header, identificador);
}

//devuelve el codigo del mensaje
char * get_codigo(char *mensaje)
{
	return string_substring(mensaje, 1, 2);
}

//obtiene el mensaje
char *get_mensaje(char *mensaje)
{
	char *payload = string_substring(mensaje, 3, 10);
	int payload1 = atoi(payload);
	free(payload);
	return string_substring(mensaje, 13, payload1);
}

char *get_mensaje_pcb(char *mensaje)
{
	char *payload = malloc(11);
	memcpy(payload,mensaje+3,10);
	payload[10]='\0';
	int payload1 = atoi(payload);
	free(payload);

	char *pcb_serializado = malloc(payload1);
	memcpy(pcb_serializado,mensaje+13,payload1);

	return pcb_serializado;
}

char *get_mensaje_escritura_info(char *mensaje, int *lar)
{
	char *payload = string_substring(mensaje, 3, 10);
	int payload1 = atoi(payload);
	free(payload);
	*lar = payload1-4;
	return string_substring(mensaje, 17, payload1-4);
}

char *get_mensaje_escritura_fd(char *mensaje)
{
	return string_substring(mensaje, 13, 4);
}
