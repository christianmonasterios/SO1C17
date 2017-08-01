/*
 * gestionar_procesos.c
 *
 *  Created on: 14/7/2017
 *      Author: utnso
 */

#include <commons/collections/list.h>
#include <stdlib.h>
#include <string.h>
#include "gestionar_procesos.h"

extern t_list *procesos;

void crear_proceso(int pid, int pag){
	t_proceso *proceso_nuevo = malloc(sizeof(t_proceso));
	proceso_nuevo->pid = pid;
	proceso_nuevo->cantPaginas = pag;
	proceso_nuevo->ultimaPagAsig = pag -1;
	list_add(procesos,proceso_nuevo);
}
int buscar_proceso(int pid){

	int i;
	int retorno = -1;
	for(i=0;i<list_size(procesos);i++){
		t_proceso *aux = list_get(procesos,i);
		if(aux->pid == pid){
			retorno = i;
		}
	}
	return retorno;
}
void eliminar_proceso(int pid){


	int pos = buscar_proceso(pid);
	list_remove_and_destroy_element(procesos,pos,(void *) destructor);
}

void destructor(t_proceso *self){
	free(self);
}

void destruir_procesos(){

	list_destroy_and_destroy_elements(procesos,(void *)destructor);

}

void actualizar_paginas(int pid,int asumar){

	int pos = buscar_proceso(pid);
	t_proceso *aux = list_get(procesos,pos);
	aux->cantPaginas += asumar;
}
void actualizar_maxnro_pagina(int pid, int numero){
	int pos = buscar_proceso(pid);
	t_proceso *aux = list_get(procesos,pos);
	aux->ultimaPagAsig += numero;
}
int ultimoNumeroPagina(int pid){

	int pos= buscar_proceso(pid);
	int retorno = -1;
	if(pos != -1){
	t_proceso *aux = list_get(procesos,pos);
	retorno = aux->ultimaPagAsig;
	}
	return retorno;
}
int cantidadPaginas(int pid){

	int pos= buscar_proceso(pid);
	int retorno = -1;
	if(pos != -1){
	t_proceso *aux = list_get(procesos,pos);
	retorno = aux->cantPaginas;
	}
	return retorno;
}

