/*
 * consola_memoria.c
 *
 *  Created on: 8/7/2017
 *      Author: utnso
 */

#include "consola_memoria.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "gestionar_procesos.h"
#include "estructuras.h"
#include "hash.h"

extern int cantMarcos;
extern int cantMarcosTablaPag;
extern int tamanioMarco;
extern char *Memoria;
extern t_cache* Cache;
extern t_LRU_cache *Cache_LRU;
extern int entradasCache;
extern t_tablaPagina* tablaPaginas;
extern int retardo;
extern t_list *procesos;
extern pthread_mutex_t mutex_cache;


int terminar_hilo;
int cantidad_de_dump_cache;
int cantidad_de_dump_estructuras;
int cantidad_de_dump_frames;




void hilo_consola_memoria(){

	cantidad_de_dump_cache = 1;
	cantidad_de_dump_estructuras = 1;
	cantidad_de_dump_frames = 1;
	terminar_hilo = 0;

	while(terminar_hilo == 0){

		char *input;
		display_menu_principal();
		scanf("%ms",&input);
		int caso = atoi(input);
		free(input);

		switch(caso){
		case 1:;
		int nuevo_retardo = 0;
		printf("\n Por favor, ingrese un nuevo valor de retardo (en milisegundos) \n\n");
		scanf("%d",&nuevo_retardo);
		retardo = nuevo_retardo;
		printf("\n Nuevo retardo:%d \n\n",retardo);
		printf("\n （＾－＾） Muchas Gracias, en unos instantes podras ver el menu principal nuevamente \n\n");

		break;
		case 2:;
		char *que_dump;
		display_menu_dump();
		scanf("%ms",&que_dump);
		int caso_dump = atoi(que_dump);
		if(caso_dump == 6){
			char *path = dump_cache();
			printf("\n El path del estado actual de la cache archivado es: %s \n\n",path);
			free(path);
		}else if(caso_dump == 7){
			char *path = dump_estructuras();
			printf("\n El path del estado actual de la Tabla de Archvos archivado es: %s \n\n",path);
			free(path);

		}else if(caso_dump == 8){
			printf("\n Si quiere ver todo el contenido de los marco ingrese -1 \n\n");
			printf("\n Si quiere marcos particulares de un proceso ingrese su PID \n\n");
			int pid = 0;
			scanf("%d",&pid);
			char *path = dump_datos(pid);
			printf("\n El path del estado actual de los datos de pid :%d archivado es: %s \n\n",pid,path);
			free(path);
		}else{
			printf("\n Lo siento,pero no conozco el comando ingresado,volveremos al menu principal \n\n");
		}
		free(que_dump);
		printf("\n （＾－＾） Muchas Gracias, en unos instantes podras ver el menu principal nuevamente \n\n");

		break;
		case 3:
			printf("\n Limpiando contenido de la Caché... \n\n");
			pthread_mutex_lock(&mutex_cache);
			limpiar_cache();
			pthread_mutex_unlock(&mutex_cache);
			printf("\n（＾－＾） Muchas Gracias, en unos instantes podras ver el menu principal nuevamente \n\n");

			break;
		case 4:
			printf("\n Tamaño de la Memoria \n\n");
			printf("\n Cantidad de Marcos : %d \n\n",cantMarcos);
			int libres = marcos_libres();
			printf("\n Cantidad de Marcos Libres : %d \n\n",libres); //calcular marcos libre
			printf("\n Cantidad de Marcos Ocupados : %d \n\n",cantMarcos-libres); // restarle los marcos libres

			printf("\n （＾－＾） Muchas Gracias, en unos instantes podras ver el menu principal nuevamente \n\n");
			break;
		case 5:;
		int pid;
		printf("\n Ingrese el PID del que quiere saber el tamaño \n\n");
		scanf("%d",&pid);
		int cantpagspid = cantidadPaginas(pid);
		int size_proceso = cantpagspid * tamanioMarco;
		printf("\n El tamaño del PID %d es :%d bytes \n",pid,size_proceso);
		printf("\n （＾－＾） Muchas Gracias, en unos instantes podras ver el menu principal nuevamente\n\n");
		break;
		case 6:
			printf("\n\n    \033[1m\x1b[45;36mಠ-ಠ ¿Qué ingresaste? ಠ-ಠ\x1b[0m  \n\n");
			break;
		}
		sleep(2);
		printf("\n\n");
	}
}


void display_menu_principal(){

	printf("----------------------------------------------\n\n");
	printf("        \033[1m\x1b[93;41mBienvenid@ （‐＾▽＾‐)\x1b[0m \n\n");
	printf("Ingrese el numero de alguna de las siguientes opciones\n");
	printf("1-	Modificar_Retardo\n");
	printf("2-	Dump\n");
	printf("3-	Flush\n");
	printf("4-	Size_Memoria\n");
	printf("5-	Size_PID\n\n");
	printf("----------------------------------------------\n\n");

}
void display_menu_dump(){

	printf("----------------------------------------------\n\n");
	printf("   \033[1m\x1b[93;41m  Bienvenid@ al menu del comando Dump（‐＾▽＾‐） \x1b[0m \n\n");
	printf("¿Sobre qué numero de opción quiere realizar el Dump?\n");
	printf("6-	Cache\n");
	printf("7-	Esctructuras_de_Memoria\n");
	printf("8-	Contenido_en_Marcos\n");
	printf("----------------------------------------------\n\n");

}

void limpiar_cache(){
	int i;
	for(i=0;i<entradasCache;i++){
		if(Cache[i].pid != -1 && Cache[i].pag != -1){
			Cache[i].pid = -1;
			Cache[i].pag = -1;
			free(Cache[i].dataFrame);
			Cache[i].dataFrame = NULL;
			}
	}
	for(i=0;i<entradasCache;i++){
		Cache_LRU[i].pag=-1;
		Cache_LRU[i].pid=-1;
		Cache_LRU[i].LRU=-1;
	}


}
char *dump_cache(){
	cantidad_de_dump_cache ++;
	char *path = string_from_format("/home/utnso/dump_cache_%d",cantidad_de_dump_cache);
	t_log* logi = log_create(path,"DUMP_CACHE",true,LOG_LEVEL_INFO);
	int i;
	char *linea;
	char *linea2;

	log_info(logi,"IMPRIMIENDO CONTENIDO MEMORIA CACHÉ");
	linea2 = string_from_format("CACHE POS - PID | PAG | DATA");
	log_info(logi,linea2);
	for(i=0; i<entradasCache ; i++){
		linea = string_from_format(" %d  -  %d | %d | %s ",i, Cache[i].pid,Cache[i].pag,Cache[i].dataFrame);
		log_info(logi,linea);
		free(linea);
	}
	free(linea2);
	log_destroy(logi);
	return path;
}
char *dump_estructuras(){

	cantidad_de_dump_estructuras ++;
	char *path = string_from_format("/home/utnso/dump_estructuras_%d",cantidad_de_dump_estructuras);
	t_log* logi = log_create(path,"DUMP_ESTRUCTURAS",true,LOG_LEVEL_INFO);
	int a;
	log_info(logi,"IMPRIMIENDO TABLA DE PAGINAS");
	char *linea;
	char *aux;

	aux = string_from_format("FRAME - ESTADO | PID | PAG");
	log_info(logi,aux);
	free(aux);
	for(a=0;a<cantMarcos;a++){
		linea = string_from_format("-%d-  %d | %d | %d",a, tablaPaginas[a].estado,tablaPaginas[a].pid,tablaPaginas[a].pag);
		log_info(logi,linea);
		free(linea);
	}

	log_info(logi,"IMPRIMIENDO PROCESOS ACTIVOS");
	int c;
	char *otra;
	for(c=0;c<list_size(procesos);c++){
		t_proceso *aux = list_get(procesos,c);
		otra = string_from_format("PID ACTIVO: %d",aux->pid);
		log_info(logi,otra);
		free(otra);
	}
	log_destroy(logi);
	return path;
}
char *dump_datos(int pid){
	cantidad_de_dump_frames ++;
	char *path = string_from_format("/home/utnso/dump_frames_%d",cantidad_de_dump_estructuras);
	t_log* logi = log_create(path,"DUMP_FRAMES",true,LOG_LEVEL_INFO);
	if(pid == -1){
		log_info(logi,"\n IMPRIMIENDO TODO EL CONTENIDO DE FRAMES PARA PROCESOS \n");
		int a;int count=1;
		for (a = cantMarcosTablaPag+1; a<cantMarcos; a ++ ) {
			char *dataFrame = malloc (tamanioMarco+1); memset(dataFrame,'\0',tamanioMarco+1);
			int pos= (a* tamanioMarco)-1;
			memcpy(dataFrame,Memoria+pos,tamanioMarco);

			char *imp = string_from_format("\n MARCO Nro:%d \n DATOS: |%s|\n",count,dataFrame);
			log_info(logi,imp);
			free(imp);
			free(dataFrame);
			count ++;
		}

	}else{
		char *aux= string_from_format("\n IMPRIMIENDO TODOS LOS FRAMES DEL PID %d \n",pid);
		log_info(logi,aux);
		free(aux);

		int maxPagPid = ultimoNumeroPagina(pid);
		printf("\n ----- MAX PAG DEL PID: %d -- PAG: %d", pid, maxPagPid);
		int i;
		for (i=0;i<=maxPagPid;i++){
		int pos = posPaginaSolicitada(pid,i);
		if ( pos != -1) {
				//Imprimir pagina
			int posframe = posFrameEnMemoria(pos);
			char * dataFrame = malloc(tamanioMarco);
			memset(dataFrame,' ',tamanioMarco);
			memcpy(dataFrame,Memoria+posframe,tamanioMarco);
			char * imp = string_from_format("|%s| - %d \n",dataFrame,i);
			log_info(logi,imp);

			free(imp);
			free(dataFrame);
		}

		}

	}
	log_destroy(logi);
	return path;
}
int marcos_libres(){
	int ret = 0;
	int a;
	for(a=0;a<cantMarcos;a++){
		if(tablaPaginas[a].estado == 0){
			ret++;
		}
	}
	return ret;
}
int calcular_paginas_de_proceso(int pid){
	int ret = 0;
	int a;
	for(a=cantMarcosTablaPag;a<cantMarcos;a++){
		if(tablaPaginas[a].pid == pid){
			ret++;
		}
	}
	return ret;
}

char *info_marco_proceso(int pid){
	int cant_pagina = ultimoNumeroPagina(pid)+1;
	char *datos = malloc(cant_pagina * tamanioMarco);
	int i;
	int desplazamiento = 0;
	for(i=0;i<cant_pagina;i++){
		int pos_hash = hash(pid,i);
		if(es_marco_correcto(pid,i,pos_hash) == 0){
			int pos_rehsh = buscar_marco_colision(pid,i,pos_hash);
			memcpy(datos,Memoria+((pos_rehsh)*tamanioMarco),tamanioMarco);
			desplazamiento += tamanioMarco;
		}else{
			memcpy(datos,Memoria+((pos_hash)*tamanioMarco),tamanioMarco);
			desplazamiento += tamanioMarco;
		}
	}
	return datos;

}
