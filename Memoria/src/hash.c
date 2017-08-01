/*
 * hash.c
 *
 *  Created on: 7/7/2017
 *      Author: utnso
 */
#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <commons/string.h>

extern int cantMarcos;

typedef struct{
	int estado;
	int pid;
	int pag;
} t_tablaPagina;

extern t_tablaPagina* tablaPaginas;
extern t_dictionary** colisiones;
extern int ultimo_frame_libre_asignado;

void inicializar_array_colisiones(){

	colisiones=malloc (sizeof(t_dictionary*)*cantMarcos);
	int i;
	for(i=0;i<cantMarcos;i++){
		colisiones[i] = dictionary_create();
	}
}

int hash(int pid,int pag){

	int marco = -1;
	char *str_pid = string_itoa(pid);
	char *str_pag = string_itoa(pag);
	char *str_full = string_from_format("%s%s",str_pid,str_pag);
	int full = atoi(str_full);
	marco = full % cantMarcos;

	free(str_pid);
	free(str_pag);
	free(str_full);

	return marco;
}

bool marco_ocupado(int marco){

	bool retorno = false;

	if(tablaPaginas[marco].estado == 1){
		retorno = true;
	}

	return retorno;
}
//int reasignar_colision(int marco,int pid,int pag){
int reasignar_colision(){
	int marco_libre = -1;
	int i;
	for(i=ultimo_frame_libre_asignado+1; i< cantMarcos; i++){
		if(tablaPaginas[i].estado == 0){
			marco_libre = i;
			return marco_libre;
		}
	}
	if (marco_libre == -1){
		for(i=0; i< ultimo_frame_libre_asignado+1; i++){
			if(tablaPaginas[i].estado == 0){
				marco_libre = i;
				return marco_libre;
			}
		}
	}
	return marco_libre;

}
void asentar_colision(int marcoFinal,int marcoGeneradorColision,int pid, int pagina){

	char *str_pid = string_itoa(pid);
	char *str_pagina = string_itoa(pagina);
	char *key = string_from_format("%s%s",str_pid,str_pagina);

	dictionary_put(colisiones[marcoGeneradorColision],key,(void *)marcoFinal);

	free(str_pid);
	free(str_pagina);
	free(key);

}
int buscar_marco_colision(int pid,int pagina,int marcoincorrecto){

	int marco = -1;
	char *str_pid = string_itoa(pid);
	char *str_pagina = string_itoa(pagina);
	char *key = string_from_format("%s%s",str_pid,str_pagina);

	void *aux = dictionary_get(colisiones[marcoincorrecto],key);
	if(aux != NULL){
		marco= (int)aux;
	}
	free(str_pagina);
	free(str_pid);
	free(key);

	return marco;
}
bool es_marco_correcto(int pid, int pagina,int marco){

	int respuesta = false;

	if(tablaPaginas[marco].pid == pid && tablaPaginas[marco].pag == pagina){
		respuesta = true;
	}
	 return respuesta;
}
int eliminar_colision(int pid,int pagina, int marcoincorrecto){

	int marco = -1;
	char *str_pid = string_itoa(pid);
	char *str_pagina = string_itoa(pagina);
	char *key = string_from_format("%s%s",str_pid,str_pagina);

	marco =(int)dictionary_get(colisiones[marcoincorrecto],key);
	dictionary_remove(colisiones[marcoincorrecto],key);

	free(str_pagina);
	free(str_pid);
	free(key);

	return marco;

}
