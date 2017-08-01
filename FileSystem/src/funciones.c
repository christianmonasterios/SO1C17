/*
 * funciones.c
 *
 *  Created on: 4/6/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <sys/mman.h>
#include <commons/bitarray.h>
#include "log.h"
#include <fcntl.h>
#include <sys/stat.h>
#include "mensaje.h"
#include "socket.h"
#include "estructuras.h"
#include "fsecundarias.h"


extern int tBloques;
extern int flagsocket;
extern t_bitarray * bitmap;
extern int cliente;
extern char * montaje;


void validar_archivo(char *mensaje)
{
	FILE *archivo;
	char *mensaje2;
	char *pathArmado;

	pathArmado = armar_path(mensaje);
	archivo = fopen(pathArmado,"r+");

	if(archivo == NULL){
		mensaje2 = armar_mensaje("F01","no");
		enviar(cliente,mensaje2,&flagsocket);
		escribir_log("El archivo no existe\n");
	}
	else {
		mensaje2 = armar_mensaje("F01","ok");
		enviar(cliente,mensaje2,&flagsocket);
		escribir_log("El archivo existe\n");
		fclose(archivo);
	}

	free(mensaje2);
	free(pathArmado);
}

void crear_archivo(char *mensaje)
{
	FILE *archivo;
	char *mensaje2;
	char *pathArmado;
	char *pathsinarchivo;
	int verificar;

	verificar = verificar_bloque();

	if(verificar != -1)
	{
		pathArmado = armar_path(mensaje);
		pathsinarchivo = sacar_archivo(pathArmado);

		char * pathmkdir= string_from_format("sudo mkdir -p %s",pathsinarchivo);
		system(pathmkdir);
		free(pathmkdir);
		archivo = fopen(pathArmado,"a+");

		if(archivo == NULL){
				mensaje2 = armar_mensaje("F02","no");
				enviar(cliente,mensaje2,&flagsocket);
				escribir_log("El archivo no se creo\n");
		}
		else {
			mensaje2 = armar_mensaje("F02","ok");
			enviar(cliente,mensaje2,&flagsocket);
			armar_archivo(archivo);
			escribir_log("El archivo se creo\n");
			fclose(archivo);

		}

		free(pathArmado);
		free(pathsinarchivo);
	}
	else{
		mensaje2 = armar_mensaje("F02","no");
		enviar(cliente,mensaje2,&flagsocket);
		escribir_log("El archivo no se creo\n");
	}

	free(mensaje2);

}

void borrar_archivo(char *mensaje)
{
	char *pathArmado;
	char *mensaje2;
	int i = 0;

	pathArmado = armar_path(mensaje);
	t_arch *archivo;

	archivo = leer_archivo(pathArmado);
	while(archivo->bloques[i] != NULL)
	{
		bitarray_clean_bit(bitmap,atoi(archivo->bloques[i]));
		i++;
	}

	remove(pathArmado);

	mensaje2 = armar_mensaje("F03","");
	enviar(cliente,mensaje2,&flagsocket);
	escribir_log("El archivo se elimino\n");

	free(pathArmado);
	free(mensaje2);
	i=0;
	while(archivo->bloques[i] != NULL){
		free(archivo->bloques[i]);
		i++;
	}
	free(archivo->bloques);
	free(archivo);
}

void obtener_datos(char *path, int offset, int size)
{
	char *mensaje; // para enviar los datos
	char *path2 = strdup(""); //para sacar cada path de bloques
	char *pathBloque;// para guardar los path hechos
	FILE *bloques; //para abrir cada archivo de bloques
	char *lectura=strdup(""); // para guardar lo que se lee
	char *lectura2 = malloc(64);
	int restoSize; // lo que falta leer
	t_arch *archivo; //guarda la info del archivo en gral
	div_t bloque ; //guarda los datos de la division para sacar los bloques y el offset
	int bloqueSig; // guarda el bloque al que hay que ir
	char *pathArmado;
	char * read;


	string_append(&path2,montaje);
	string_append(&path2,"/Bloques/");

	pathArmado = armar_path(path);

	archivo = leer_archivo(pathArmado);
	bloque = div(offset,tBloques);
	bloqueSig = bloque.quot;

	pathBloque = armar_pathBloque(path2,bloqueSig,archivo);

	bloques = fopen(pathBloque,"r");
	free(pathBloque);
	fseek(bloques,bloque.rem,SEEK_SET);
	restoSize = size;

	while(string_length(lectura)<size)
	{
		if(restoSize <= 64-bloque.rem)
		{
			fread(lectura2,sizeof(char),restoSize,bloques);
			read = string_substring(lectura2,0,restoSize);
			string_append(&lectura, read);
			restoSize = size;

		} // preguntar estructura de los bloques.bin
		else{

			fread(lectura2,sizeof(char),64-bloque.rem,bloques);
			read = string_substring(lectura2,0,64-bloque.rem);
			string_append(&lectura,read);

			free(read);

			fclose(bloques);

			restoSize = size - string_length(lectura);

			bloqueSig ++;
			if(archivo->bloques[bloqueSig] != '\0')
			{
				pathBloque = armar_pathBloque(path2,bloqueSig,archivo);
				bloques =fopen(pathBloque,"r");
				free(pathBloque);
				bloque.rem = 0;
			}
			else{
				restoSize = size + 1;
			}
		}
	}

	if(restoSize == size){
		mensaje = armar_mensaje("F04",lectura);
		enviar(cliente,mensaje,&flagsocket);
		string_append(&lectura,"\n");
		escribir_log_compuesto("Se realizo la lectura: ", lectura);
	}
	else {
		mensaje = armar_mensaje("F06","");
		enviar(cliente,mensaje,&flagsocket);
		escribir_log("No se pudo realizar la lectura\n");
	}


	fclose(bloques);
	free(read);
	free(mensaje);
	free(lectura);
	free(lectura2);
	int i=0;
	while(archivo->bloques[i] != NULL){
		free(archivo->bloques[i]);
		i++;
	}
	free(archivo->bloques);
	free(archivo);
	free(pathArmado);

}

void guardar_datos(char *path, int offset, int size, char *buffer)
{
	char *mensaje; // para enviar los datos
	char *path2 = strdup(""); //para sacar cada path de bloques
	char *pathBloque;// para guardar los path hechos
	FILE *bloques; //para abrir cada archivo de bloques
	t_arch *archivo; //guarda la info del archivo en gral
	div_t bloque ; //guarda los datos de la division para sacar los bloques y el offset
	int bloqueSig; // guarda el bloque al que hay que ir
	int guardado = 0;
	char *bloques_agregados = strdup("");
	char *pathArmado;
	char *bloques_final;
	int sizeAguardar = 0;
	int flag=0;
	//int stop;
	char *write;
	int bit;
	int i = 0;

	string_append(&path2,montaje);
	string_append(&path2,"/Bloques/");

	pathArmado = armar_path(path);

	archivo = leer_archivo(pathArmado);
	while(archivo->bloques[i] != NULL) i++;
	bloque = div(offset,tBloques);
	bloqueSig = bloque.quot;

	if(bloqueSig >= i)
	{
		bloqueSig = verificar_bloque();
		if(bloqueSig != -1){
			bit=agregar_bloque(bloqueSig);
		}
		if(bloqueSig != -1 && bit != -1)
		{
			pathBloque = armar_pathBloqueNuevo(path2,bloqueSig);
			bloques =fopen(pathBloque,"r+");
			bloque.rem = 0;

			string_append(&bloques_agregados,",");
			string_append(&bloques_agregados,string_itoa(bloqueSig));

			if(flag == 0){
				flag = 1;
				//stop = guardado;
			}

		}
		else{
			guardado = size + 1;
		}

	}
	else{
		pathBloque = armar_pathBloque(path2,bloqueSig,archivo);

		bloques = fopen(pathBloque,"r+");
		fseek(bloques,bloque.rem,SEEK_SET);
	}


	if(offset < archivo->tamanio)
	{
		while (guardado < size)
		{
			if((size - guardado) <= (64-bloque.rem))
			{
				write = string_substring(buffer,guardado,size - guardado);
				fwrite(write,sizeof(char),string_length(write),bloques);
				//fputs(write,bloques);
				guardado = size;
				fclose(bloques);
				flag=2;
				free(write);
				/*if(flag == 1){
					sizeAguardar = guardado - stop;
				}*/

			}
			else
			{
				write = string_substring(buffer,guardado,64-bloque.rem);
				fwrite(write,sizeof(char),string_length(write),bloques);
				//fputs(write,bloques);
				guardado += 64-bloque.rem;

				fclose(bloques);

				free(write);

				if(archivo->bloques[bloqueSig+1] != NULL /*'\0'*/)
				{
					bloqueSig ++;
					pathBloque = armar_pathBloque(path2,bloqueSig,archivo);
					bloques =fopen(pathBloque,"r+");
					bloque.rem = 0;
					flag=2;
				}
				else
				{
					bloqueSig = verificar_bloque();
					if(bloqueSig != -1){
						bit=agregar_bloque(bloqueSig);
					}

					if(bloqueSig != -1 && bit != -1)
					{
						pathBloque = armar_pathBloqueNuevo(path2,bloqueSig);
						bloques =fopen(pathBloque,"r+");
						bloque.rem = 0;

						string_append(&bloques_agregados,",");
						string_append(&bloques_agregados,string_itoa(bloqueSig));

						/*if((size - guardado) > (64-bloque.rem))
						{
							string_append(&bloques_agregados,",");
						}*/

						flag = 2;

					 }
					 else
					 {
						 guardado = size + 1;

						 /*if(flag == 0){
							 guardado = size + 1;
						 }
						 else{
							 sizeAguardar = guardado - stop;
							 guardado = size + 1;
						 }*/
					 }
				}
			}
		}
	}

	else
	{
		while(guardado < size)
		{
			if((size - guardado) <= (64-bloque.rem))
			{
				write = string_substring(buffer,guardado,size - guardado);
				//fwrite(write,sizeof(char),string_length(write),bloques);
				fputs(write,bloques);
				//sizeAguardar += size - guardado;
				guardado = size;
				flag =1;
				fclose(bloques);
				free(write);

			} // preguntar estructura de los bloques.bin
			else
			{
				write = string_substring(buffer,guardado,64-bloque.rem);
				//fwrite(write,sizeof(char),string_length(write),bloques);
				fputs(write,bloques);
				guardado += 64-bloque.rem;

				fclose(bloques);

				free(write);

				bloqueSig = verificar_bloque();
				if(bloqueSig != -1){
					bit=agregar_bloque(bloqueSig);
				}

				if(bloqueSig != -1 && bit != -1)
				{
					pathBloque = armar_pathBloqueNuevo(path2,bloqueSig);
					bloques =fopen(pathBloque,"r+");
					bloque.rem = 0;

					string_append(&bloques_agregados,",");
					string_append(&bloques_agregados,string_itoa(bloqueSig));

					/*if((size - guardado) > (64-bloque.rem))
					{
						string_append(&bloques_agregados,",");
					}*/

					if(flag == 0){
						flag = 1;
						//stop = guardado;
					}
				}
				else{
					guardado = size + 1;
				}
			}
		}
	}

	if(flag == 1){
		archivo->tamanio += size;
		bloques_final = crear_string_bloques(archivo->bloques, bloques_agregados);
		modificar_archivo(pathArmado,archivo->tamanio,bloques_final);
		free(bloques_final);
	}

	if(flag == 2){
		archivo->tamanio = offset + size;
		bloques_final = crear_string_bloques(archivo->bloques, bloques_agregados);
		modificar_archivo(pathArmado,archivo->tamanio,bloques_final);
		free(bloques_final);
	}


	if(guardado == size){
		mensaje = armar_mensaje("F05","ok");
		enviar(cliente,mensaje,&flagsocket);
		escribir_log("Se realizaron cambios en el archivo\n");
	}
	else{
		mensaje = armar_mensaje("F05","no");
		enviar(cliente,mensaje,&flagsocket);
		escribir_log("No se pudieron realizar cambios en el archivo\n");
	}


	free(path2);
	free(mensaje);
	free(pathBloque);
	i=0;
	while(archivo->bloques[i] != NULL){
		free(archivo->bloques[i]);
		i++;
	}
	free(archivo->bloques);
	free(archivo);
	free(bloques_agregados);
	free(pathArmado);

}
