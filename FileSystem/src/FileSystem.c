/*
 ============================================================================
 Name        : FileSystem.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket.h"
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include "funciones.h"
#include "archivos.h"
#include "log.h"
#include <commons/log.h>
#include "mensaje.h"
#include <commons/bitarray.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <commons/collections/dictionary.h>
#include "estructuras.h"
#include "fsecundarias.h"

int puerto;
char *ip;
int cliente;
char *montaje;
int flagsocket;
int socketfs;
t_log * log_;
int tBloques;
int cantBloques;
char *magic_number;
t_bitarray * bitmap;
char* posicion;
int bitm;

struct stat mystat;

int verificarHS(char *handshake);
void handshake1();
void reservar_memoria();
void liberar_memoria();
void actualizar_bitmap();


int main(int argc, char *argv[])
{
	int metadata;
	int flag = 0;

	reservar_memoria();
	archivoDeCofiguracion(argv[1]);

	metadata = leer_metadata();
	if (metadata == -1) goto finalizar;



	bitm = abrir_bitmap();
	if(bitm == -1) goto finalizar;

	posicion = malloc(cantBloques);

	flagsocket=0;
	socketfs =iniciar_socket_server(ip,puerto,&flagsocket);

	handshake1();

	while(flag == 0)
	{
		char *mensaje;
		char *codigo;
		char *mensaje2;

		mensaje = recibir(cliente,&flagsocket);
		codigo = get_codigo(mensaje);

		switch(atoi(codigo))
		{
			case 11:
				mensaje2 = get_mensaje(mensaje);
				validar_archivo(mensaje2);
				free(mensaje2);
				break;
			case 12:
				mensaje2 = get_mensaje(mensaje);
				crear_archivo(mensaje2);
				free(mensaje2);
				break;
			case 13:
				mensaje2 = get_mensaje(mensaje);
				borrar_archivo(mensaje2);
				free(mensaje2);
				break;
			case 14: ;
				t_datos * est;
				mensaje2 = get_mensaje(mensaje);
				est =recuperar_datos(codigo,mensaje2);
				obtener_datos(est->path,est->offset,est->size);
				free(est->buffer);
				free(est->path);
				free(est);
				free(mensaje2);
				break;
			case 15: ;
				t_datos * estruct;
				mensaje2 = get_mensaje(mensaje);
				estruct = recuperar_datos(codigo,mensaje2);
				guardar_datos(estruct->path,estruct->offset,estruct->size,estruct->buffer);
				free(mensaje2);
				free(estruct->buffer);
				free(estruct->path);
				free(estruct);
				break;
			default:
				escribir_log("Mensaje incorrecto");
				flag =1;
				break;

		}
		free(mensaje);
		free(codigo);

		actualizar_bitmap();
	}

	if(metadata == -1 || bitm == -1){
		finalizar: escribir_log("Error leyendo archivos iniciales");
	}

	liberar_memoria();
	return EXIT_SUCCESS;
}
					////// AUXILIARES //////

void handshake1()
{
	char *handshake;
	int esKernel=0;
	//char *mensaje;
	while(esKernel == 0)
	{
		cliente = escuchar_conexiones(socketfs,&flagsocket);
		handshake = recibir(cliente,&flagsocket);
		if (verificarHS(handshake)== 1)
		{
			esKernel = 1;
			//mensaje = armar_mensaje("F00","");
			//enviar(cliente,mensaje,&flagsocket);
			//free(mensaje);
		}else{
			cerrar_conexion(cliente);
			printf("intruso no kernel eliminado \n");
			escribir_log("Proceso no Kernel eliminado\n");
		}
		free(handshake);
	}
	printf("KERNEL CONECTADO \n");
	escribir_log("Se conecto el Kernel\n");
}

void reservar_memoria()
{
	montaje = strdup("");
	ip = strdup("");
	magic_number = strdup("");
	crear_archivo_log("/home/utnso/log_fs.txt");
}

void liberar_memoria()
{
	free(montaje);
	free(ip);
	free(magic_number);
	if(bitm != -1){
		memcpy(posicion,bitmap,mystat.st_size);
		msync(posicion,mystat.st_size,MS_SYNC);
		munmap(posicion,mystat.st_size);
		bitarray_destroy(bitmap);
	}
	free(posicion);
	liberar_log();
}

int verificarHS(char *handshake)
{
	char *header = get_header(handshake);
	if(!strcmp(header,"K"))
	{
		free(header);
		return 1;
	}
	else{
		free(header);
		return 0;
	}

}
void actualizar_bitmap(){

	memcpy(posicion,bitmap,mystat.st_size);
	msync(posicion,mystat.st_size,MS_SYNC);

}
