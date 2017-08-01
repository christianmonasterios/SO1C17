/*
 * archivos.c
 *
 *  Created on: 6/6/2017
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <commons/config.h>
#include <commons/string.h>
#include <sys/mman.h>
#include <commons/bitarray.h>
#include "log.h"
#include <fcntl.h>
#include <sys/stat.h>


extern char *montaje;
extern int puerto;
extern char *ip;
extern int tBloques;
extern int cantBloques;
extern char *magic_number;
extern t_bitarray * bitmap;
extern struct stat mystat;
extern char * posicion;

void archivoDeCofiguracion(char* argv)
{
	t_config *configuracion;
	printf("ruta archivo de configuacion: %s \n", argv);
	configuracion = config_create(argv);
	puerto = config_get_int_value(configuracion, "PUERTO");
	string_append(&montaje, config_get_string_value(configuracion, "PUNTO_MONTAJE"));
	string_append(&ip, config_get_string_value(configuracion, "IP"));
	escribir_log_compuesto("Valor IP: ", ip);
	escribir_log_con_numero("Valor puerto para conexion del Kernel: ", puerto);
	escribir_log_compuesto("Valor punto montaje FS: ",montaje);

	config_destroy(configuracion);
}

int leer_metadata()
{
	char *ruta = strdup("");
	t_config *configuracion;

	string_append(&ruta,montaje);
	string_append(&ruta,"/Metadata/Metadata.bin");

	configuracion = config_create(ruta);

	tBloques = config_get_int_value(configuracion, "TAMANIO_BLOQUES");
	cantBloques = config_get_int_value(configuracion, "CANTIDAD_BLOQUES");
	string_append(&magic_number, config_get_string_value(configuracion, "MAGIC_NUMBER"));

	if(strcmp(magic_number, "SADICA"))
	{
		config_destroy(configuracion);
		escribir_log("No es SADICA");
		free(ruta);
		return -1;
	}

	escribir_log_con_numero("Tamanio de Bloques: ", tBloques);
	escribir_log_con_numero("Cantidad de bloques: ", cantBloques);
	string_append(&magic_number, "\n");
	escribir_log_compuesto("Magic Number: ",magic_number);

	free(ruta);
	config_destroy(configuracion);
	return 0;

}

int abrir_bitmap()
{
	char *ruta = strdup("");

	string_append(&ruta,montaje);
	string_append(&ruta,"/Metadata/Bitmap.bin");

	int fdbitmap = open(ruta,O_RDWR);
	free(ruta);
	if(fdbitmap==0){
		escribir_log("no abrio el bitmap\n");
		close(fdbitmap);
		return -1;
	}

	fstat(fdbitmap,&mystat);

	posicion = mmap(0,mystat.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fdbitmap,0);
	if(posicion == MAP_FAILED){
		escribir_log("error en mmap\n");
		fprintf(stderr, "mmap failed: %s\n", strerror(errno));
		close(fdbitmap);
		return -1;
	}
	bitmap = bitarray_create_with_mode(posicion,mystat.st_size,LSB_FIRST);


	return 0;

}
