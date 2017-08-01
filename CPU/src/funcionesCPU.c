/*
 * mensajesCPU.c
 *
 *  Created on: 30/4/2017
 *      Author: utnso
 */

#include "funcionesCPU.h"

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "estructuras.h"
#include "log.h"
#include "socket.h"

extern int tam_pagina_memoria;
extern t_PCB_CPU* pcb;

t_dictionary* armarDiccionarioEtiquetas(char* etiquetas_serializadas){
	t_dictionary* dicc = dictionary_create();
	int n=0;
	int cantEtiquetas=0;
	int desplazamiento = sizeof(int);
	memcpy(&cantEtiquetas,etiquetas_serializadas+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);

	while(n<cantEtiquetas){

		int lengkey=0;char *key_aux;int pasar_pc=0;
		memcpy(&lengkey,etiquetas_serializadas+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		key_aux= string_substring(etiquetas_serializadas,desplazamiento,lengkey);
		desplazamiento += lengkey;
		memcpy(&pasar_pc,etiquetas_serializadas+desplazamiento,sizeof(int));
		dictionary_put(dicc,key_aux,(void *)pasar_pc);
		desplazamiento += sizeof(int);
		free(key_aux);
		n++;

	}
	return dicc;

}
char *serializarPCB_CPUKer (t_PCB_CPU* pcb1,int * devolveme){
	char *retorno;
	//int sizeretorno = tamaño_PCB(pcb);
	int size_retorno = sizeof(int)*5; //(int) PID,PC,CANT_PAGINAS,SP,EXIT_CODE
	// TAMAÑO_SENTENCIAS_SERIALIZADAS + SENTENCIAS_SERIALIZADAS (c/sentencias : (int)inicio,(int)offset)
	// en la serializacion de etiquetas como en el indice hay una extra, de control, con valores negativos
	int cantidad_sentencias=0;
	while(pcb1->in_cod[cantidad_sentencias].offset_inicio != -1 && pcb1->in_cod[cantidad_sentencias].offset_fin != -1 ){
		cantidad_sentencias++;
	}
	int size_in_sentencias= (cantidad_sentencias+1)*sizeof(t_sentencia);
	size_retorno += size_in_sentencias + sizeof(int);
	// SIZE TOTAL DEL INDICE DE ETIQUETAS (SERIALIZACION DE UN DICCIONARIO)
	int size_in_et = 0; memcpy(&size_in_et,pcb1->in_et,4);
	size_retorno += size_in_et;
	//(int) SIZE TOTAL INDICE DE STACK + (int) CANT_ELEMENTOS +T_STACK_ELEMENT SERIALIZADO
	//(c/u t_stack_element: (int) pos+(int)retPos + (13 bytes t_memoria serializada) retVar + (int) size_argumentos + (13bytes*nElemetos) args + (int) size_vars + (13bytes) vars)
	//(c/u t_memoria : retVar, vars, args: (char) ID, (int) pag,(int) offset,(int) size)
	int size_in_stack = 0;
	int n=0; int tam_stack = list_size(pcb1->in_stack);
	while (n < tam_stack){
		t_stack_element* aux = list_get(pcb1->in_stack,n);
		size_in_stack += 4* sizeof(int) + sizeof(t_memoria)+ sizeof(t_memoria)*( list_size(aux->args) + list_size(aux->vars) );
		n++;
	}
	size_retorno += size_in_stack + 2 * sizeof(int);
	retorno = malloc(size_retorno+1);
	bzero(retorno,size_retorno+1);
	*devolveme = size_retorno+1;
	int desplazamiento = 0;

	// 4 BYTES C/U PARA: SIZE_TOTAL_MENSAJE,PID,PC,CANT_PAGINAS,SP,EXIT_CODE,QUANTUM,QUANTUM_SLEEP
	memcpy(retorno+desplazamiento,&pcb1->PID,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(retorno+desplazamiento,&pcb1->PC,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(retorno+desplazamiento,&pcb1->cant_pag,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(retorno+desplazamiento,&pcb1->SP,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(retorno+desplazamiento,&pcb1->exit_code,sizeof(int));
	desplazamiento += sizeof(int);

	// 4 BYTES PARA TAMAÑO_SENTENCIAS_SERIALIZADAS
	memcpy(retorno +desplazamiento,&size_in_sentencias,sizeof(int));
	desplazamiento += sizeof(int);

	// SERIALIZO SENTENCIAS
	n=0;
	while(pcb1->in_cod[n].offset_inicio != -1 &&pcb1->in_cod[n].offset_fin != -1 ){
		memcpy(retorno+desplazamiento, &pcb1->in_cod[n].offset_inicio,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento, &pcb1->in_cod[n].offset_fin,sizeof(int));
		desplazamiento += sizeof(int);
		n++;
	}
	memcpy(retorno+desplazamiento, &pcb1->in_cod[n].offset_fin,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(retorno+desplazamiento, &pcb1->in_cod[n].offset_fin,sizeof(int));
	desplazamiento += sizeof(int);
	// AGREGO EL INDICE_ETIQUETAS (DICCIONARIO DE ETIQUETAS SERIALIZADO)
	memcpy(retorno+desplazamiento,pcb1->in_et,size_in_et);
	desplazamiento += size_in_et;

	// 4 BYTES C/U PARA : SIZE_IN_STACK, CANT_ELEMENTOS_STACK
	memcpy(retorno+desplazamiento,&size_in_stack,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(retorno+desplazamiento,&tam_stack,sizeof(int));
	desplazamiento += sizeof(int);

	n=0;
	while (n < tam_stack){
		t_stack_element* aux = list_get(pcb1->in_stack,n);
		memcpy(retorno+desplazamiento,&aux->pos,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&aux->retPos,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&aux->retVar.ID,sizeof(char));
		desplazamiento += sizeof(char);
		memcpy(retorno+desplazamiento,&aux->retVar.offset,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&aux->retVar.pag,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&aux->retVar.size,sizeof(int));
		desplazamiento += sizeof(int);
		int n_args=0;int n_vars= 0;
		n_args= list_size(aux->args);
		n_vars = list_size(aux->vars);
		int c=0;
		memcpy(retorno+desplazamiento,&n_args,sizeof(int));
		desplazamiento += sizeof(int);
		while(c<n_args){
			t_memoria* aux2 = list_get(aux->args,c);
			memcpy(retorno+desplazamiento,&aux2->ID,sizeof(char));
			desplazamiento += sizeof(char);
			memcpy(retorno+desplazamiento,&aux2->offset,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(retorno+desplazamiento,&aux2->pag,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(retorno+desplazamiento,&aux2->size,sizeof(int));
			desplazamiento += sizeof(int);
			c++;
		}
		c=0;
		memcpy(retorno+desplazamiento,&n_vars,sizeof(int));
		desplazamiento += sizeof(int);
		while(c<n_vars){
			t_memoria* aux2 = list_get(aux->vars,c);
			memcpy(retorno+desplazamiento,&aux2->ID,sizeof(char));
			desplazamiento += sizeof(char);
			memcpy(retorno+desplazamiento,&aux2->offset,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(retorno+desplazamiento,&aux2->pag,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(retorno+desplazamiento,&aux2->size,sizeof(int));
			desplazamiento += sizeof(int);
			c++;
		}


		n++;
	}
	retorno[size_retorno]='\0';
	return retorno;


}
t_PCB_CPU* deserializarPCB_KerCPU (char * pcbserializado){
	int desplazamiento =0;
	int size_in_cod = 0; int size_in_et = 0;
	int size_in_stack = 0; int cant_stack = 0;

	t_PCB_CPU *pcb2 = malloc(sizeof(t_PCB_CPU));
	memcpy(&pcb2->PID,pcbserializado,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&pcb2->PC,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(&pcb2->cant_pag,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&pcb2->SP,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&pcb2->exit_code,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&pcb2->quantum,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&pcb2->quantum_sleep,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	pcb2->algoritmo = malloc(2);
	memcpy(pcb2->algoritmo,pcbserializado+desplazamiento,sizeof(char)*2);
	desplazamiento += sizeof(char)*2;
	memcpy(&size_in_cod,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	pcb2->in_cod = malloc(sizeof(t_sentencia) *(size_in_cod / sizeof(t_sentencia)));

	int n=0;
	while(n < (size_in_cod / sizeof(t_sentencia))){
		t_sentencia aux;
		memcpy(&aux.offset_inicio,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&aux.offset_fin,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		pcb2->in_cod[n]= aux;
		n++;
	}

	memcpy(&size_in_et,pcbserializado+desplazamiento,sizeof(int));
	pcb2->in_et = malloc(size_in_et);
	memcpy(pcb2->in_et,pcbserializado+desplazamiento,size_in_et);
	pcb2->dicc_et = armarDiccionarioEtiquetas(pcb2->in_et);
	desplazamiento += size_in_et;
	memcpy(&size_in_stack,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&cant_stack,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	pcb2->in_stack = list_create();
	n=0;
	while(n < cant_stack){
		t_stack_element* aux = malloc(sizeof(t_stack_element));
		memcpy(&aux->pos,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&aux->retPos,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		t_memoria aux2;
		memcpy(&aux2.ID,pcbserializado+desplazamiento,sizeof(char));
		desplazamiento += sizeof(char);
		memcpy(&aux2.offset,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&aux2.pag,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&aux2.size,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		aux->retVar = aux2;
		int n_args=0; int n_vars=0;int c=0;
		memcpy(&n_args,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		aux->args = list_create();
		while(c<n_args){
			t_memoria* aux3 = malloc(sizeof(t_memoria));
			memcpy(&aux3->ID,pcbserializado+desplazamiento,sizeof(char));
			desplazamiento += sizeof(char);
			memcpy(&aux3->offset,pcbserializado+desplazamiento,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(&aux3->pag,pcbserializado+desplazamiento,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(&aux3->size,pcbserializado+desplazamiento,sizeof(int));
			desplazamiento += sizeof(int);
			list_add(aux->args,aux3);
			c++;
		}
		memcpy(&n_vars,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		aux->vars = list_create();
		c=0;
		while(c<n_vars){
			t_memoria* aux3 = malloc(sizeof(t_memoria));
			memcpy(&aux3->ID,pcbserializado+desplazamiento,sizeof(char));
			desplazamiento += sizeof(char);
			memcpy(&aux3->offset,pcbserializado+desplazamiento,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(&aux3->pag,pcbserializado+desplazamiento,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(&aux3->size,pcbserializado+desplazamiento,sizeof(int));
			desplazamiento += sizeof(int);
			list_add(aux->vars,aux3);
			c++;
		}
		list_add(pcb2->in_stack,aux);
		n++;
	}
	return pcb2;
}


int handshakeKernel(int socketKP){
	char *handshake = malloc(3);
	int control=0;
	recibir(socketKP,&control,handshake,3);
	if (control !=0 ){
		escribir_log("error recibiendo mensaje del Kernel",2);
	}else {

		if(strncmp(handshake,"K00",3) == 0){

			escribir_log("mensaje de conexion con Kernel recibido",1);
			char *k= strdup("P000000000000");
			enviar(socketKP,k,&control,13);
			free(k);
			if(control != 0){
				escribir_log("error enviando mensaje al Kernel",2);
			}
			escribir_log("handshake Kernel realizado exitosamente",1);
		}
	}
	free (handshake);
	return control;
}
int handshakeMemoria(int socketMP){
	char *handshake = malloc(13);
	memset(handshake,'\0',13);
	int control=0; int tamanopag=0;
	memcpy(handshake,"P000000000000",13);
	enviar(socketMP,handshake,&control,13);

	recibir(socketMP,&control,handshake,13);
	if (control !=0 ){
		escribir_log("error recibiendo mensaje de la Memoria",2);
	}else {

		if(strncmp(handshake,"M00",3) == 0){
			if(control != 0){
				escribir_log("error enviando mensaje al Memoria",2);
			}
			escribir_log("handshake Memoria realizado exitosamente",1);
			char *tammensaje= string_substring(handshake,3,10);
			int tamimensaje= atoi(tammensaje);
			free(tammensaje);
			char *tampag = malloc(tamimensaje+1);
			memset(tampag,'\0',tamimensaje+1);
			recibir(socketMP,&control,tampag,tamimensaje);
			tamanopag = atoi(tampag);
			free(tampag);


		}
	}
	free (handshake);
	tam_pagina_memoria = tamanopag;
	char *tmp= string_from_format("tamanio pagina %d",tam_pagina_memoria);
	escribir_log(tmp,1);
	free(tmp);
	return control;

}
int calcular_offset(t_list* args,t_list* vars){
	int nuevo_offset;
	// comparar el ultimo offset de cada lista
	if(list_size(args)!=0 && list_size(vars)!=0){
		t_memoria* ultimo_arg = list_get(args,list_size(args)-1);
		t_memoria* ultima_var = list_get(vars,list_size(vars)-1);
		if(ultimo_arg->offset < ultima_var->offset){
			nuevo_offset = ultima_var->offset+4;
		}else{
			nuevo_offset = ultimo_arg->offset + 4;
		}
		// comparar segun lista no vacia
	}else{
		// no hay variables y si hay argumentos
		if(list_size(args)!=0 && list_size(vars)==0){
			t_memoria* ultimo_arg = list_get(args,list_size(args)-1);
			nuevo_offset = ultimo_arg->offset + 4;
			// no hay argumentos y si hay variables
		}if(list_size(args)==0 && list_size(vars)!=0){
			t_memoria* ultima_var = list_get(vars,list_size(vars)-1);
			nuevo_offset = ultima_var->offset + 4;
			// no hay ni variables ni argumentos
		}else if(list_size(args)==0 && list_size(vars)==0){

			if(list_size(pcb->in_stack)>1){
				t_stack_element* anterior = list_get(pcb->in_stack,pcb->SP - 1);

				nuevo_offset = calcular_offset(anterior->args,anterior->vars);
			}else{
				nuevo_offset = 0;
			}

		}
	}

	return nuevo_offset;
}
int buscar_offset_variable(t_list* vars,char id){
	int retorno=-1;
	int iterador=0;
	while(iterador < list_size(vars)){
		t_memoria* var_aux = list_get(vars,iterador);
		if(var_aux->ID == id){
			retorno= var_aux->offset;
		}
		iterador++;
	}
	return retorno;

}
int calcular_pagina(int offset,int paginas){
	int pagina= offset / tam_pagina_memoria;
	return pagina+paginas;
}
void stack_destroy(t_stack_element *self) {
	list_destroy_and_destroy_elements(self->args,(void *) t_memoria_destroy);

	list_destroy_and_destroy_elements(self->vars,(void *) t_memoria_destroy);

	free(self);

}

void t_memoria_destroy(t_memoria *self){
	free(self);
}
int calcular_offset_respecto_pagina(int offset){
	return offset % tam_pagina_memoria;
}

bool linea_esta_dividida(int offset, int largo){

	bool division = false;

	if(offset + largo > tam_pagina_memoria){
		division = true;
	}

	return division;
}

int recibir_no_bloqueante(int socket_receptor, int *controlador,void *buff,int size){
	int ret;

		*controlador = 1;

		if ((ret = recv(socket_receptor, buff, size,MSG_DONTWAIT)) <= 0)
		{
			*controlador = -1;
		}
		return ret;
}
