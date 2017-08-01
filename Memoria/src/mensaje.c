/*
 * mensaje.c

 *
 *  Created on: 23/4/2017
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>

char *armar_mensaje(char *identificador, char *mensaje)
{
	char *resultado = strdup(identificador);
	char *payload_char = string_itoa(string_length(mensaje));
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
	char *resultado = malloc(13+sizepcb+1);
	memset(resultado,'/0',14+sizepcb);
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

int comparar_header(char *identificador, char *mensaje)
{
	return !strcmp(string_substring(mensaje, 0, 1), identificador);
}

//devuelve el codigo del mensaje
char *get_codigo(char *mensaje)
{
	return string_substring(mensaje, 1, 2);
}

//obtiene el mensaje
char * get_mensaje(char *mensaje)
{
	char *payload = string_substring(mensaje, 3, 10);
	int payload1 = atoi(payload);
	free(payload);
	return string_substring(mensaje, 13, payload1);
}
