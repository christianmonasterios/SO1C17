/*
 * metadata.c
 *
 *  Created on: 31/5/2017
 *      Author: utnso
 */

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <parser/metadata_program.h>
#include "estructuras.h"
#include "metadata.h"

t_sentencia* armarIndiceCodigo (char *codigoPrograma){
	t_metadata_program *metadata = metadata_desde_literal(codigoPrograma);
	t_sentencia* sentencias = malloc( sizeof(t_sentencia) * (metadata->instrucciones_size+1));

	int i ;
	for(i=0; i < metadata->instrucciones_size; i++){
		t_sentencia nueva;
		nueva.offset_inicio = metadata->instrucciones_serializado[i].start;
		nueva.offset_fin = metadata->instrucciones_serializado[i].offset;
		sentencias[i]=nueva;
	}
	t_sentencia nueva;
	nueva.offset_inicio = -1;
	nueva.offset_fin = -1;
	sentencias[i]= nueva;
	metadata_destruir(metadata);
	return sentencias;
}

char* armarIndiceEtiquetas(char *codigoPrograma){
	t_metadata_program *metadata = metadata_desde_literal(codigoPrograma);
	int cantidadEtiquetas= metadata-> cantidad_de_funciones+ metadata->cantidad_de_etiquetas;
	int largos[cantidadEtiquetas];
	int n=0;
	int largoserializado=0;
	while(n < cantidadEtiquetas){
		if(n==0){
			largos[n]= strlen(metadata->etiquetas);

		}else{
			largos[n]= strlen(metadata->etiquetas+largos[n-1]+n*5);
		}
		largoserializado =largoserializado+ largos[n];
		n++;
	}

	largoserializado = largoserializado+ (sizeof(int) *(cantidadEtiquetas)*2)+2*sizeof(int);
	char* etiquetas= malloc(largoserializado);
	n=0;int desplazamiento =0;
	memcpy(etiquetas+desplazamiento,&largoserializado,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(etiquetas+desplazamiento,&cantidadEtiquetas,sizeof(int));
	desplazamiento += sizeof(int);

	while(n < cantidadEtiquetas){
		char* key_aux;
		if(n==0){
			key_aux= string_substring(metadata->etiquetas,0,largos[n]);
		}else{
			key_aux= string_substring(metadata->etiquetas,largos[n-1]+n*5,largos[n]);
		}
		int k= largos[n];
		memcpy(etiquetas+desplazamiento,&k,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(etiquetas+desplazamiento,key_aux,k);
		desplazamiento += k;
		int z = metadata_buscar_etiqueta(key_aux,metadata->etiquetas,metadata->etiquetas_size);
		memcpy(etiquetas+desplazamiento,&z,sizeof(int));
		desplazamiento += sizeof(int);
		free(key_aux);
		n++;
	}

	metadata_destruir(metadata);
	return etiquetas;
}

t_list* armarIndiceStack (char *codigoPrograma){
	t_list *lista = list_create();
	t_stack_element * inicial= malloc(sizeof(t_stack_element));
	inicial->args = list_create();
	inicial->vars = list_create();
	inicial->pos = 0;
	inicial->retPos = 0;
	inicial->retVar.ID = '\0';
	inicial->retVar.offset = 0;
	inicial->retVar.pag = 0;
	inicial->retVar.size = 0;
	list_add(lista,inicial);
	return lista;
}

char* serializarPCB_KerCPU(t_PCB* pcb,char * algoritmo,int quantum,int quantum_sleep,int *devolveme){
	char *retorno;
		//int sizeretorno = tamaño_PCB(pcb);
		int size_retorno = sizeof(int)*7+ 2* sizeof(char); //(int) SIZEOF MENSAJE,PID,PC,CANT_PAGINAS,SP,EXIT_CODE,QUANTUM,QUANTUM_SLEEP - (2-char) ALGORITMO
		// TAMAÑO_SENTENCIAS_SERIALIZADAS + SENTENCIAS_SERIALIZADAS (c/sentencias : (int)inicio,(int)offset)
		// en la serializacion de etiquetas como en el indice hay una extra, de control, con valores negativos
		int cantidad_sentencias=0;
		while(pcb->in_cod[cantidad_sentencias].offset_inicio != -1 &&pcb->in_cod[cantidad_sentencias].offset_fin != -1 ){
			cantidad_sentencias++;
		}
		int size_in_sentencias= (cantidad_sentencias+1)*sizeof(t_sentencia);
		size_retorno += size_in_sentencias + sizeof(int);
		// SIZE TOTAL DEL INDICE DE ETIQUETAS (SERIALIZACION DE UN DICCIONARIO)
		int size_in_et = 0; memcpy(&size_in_et,pcb->in_et,4);
		size_retorno += size_in_et;
		//(int) SIZE TOTAL INDICE DE STACK + (int) CANT_ELEMENTOS +T_STACK_ELEMENT SERIALIZADO
		//(c/u t_stack_element: (int) pos+(int)retPos + (13 bytes t_memoria serializada) retVar + (int) size_argumentos + (13bytes*nElemetos) args + (int) size_vars + (13bytes) vars)
		//(c/u t_memoria : retVar, vars, args: (char) ID, (int) pag,(int) offset,(int) size)
		int size_in_stack = 0;
		int n=0; int tam_stack = list_size(pcb->in_stack);
		while (n < tam_stack){
			t_stack_element* aux = list_get(pcb->in_stack,n);
			size_in_stack += 4* sizeof(int) + sizeof(t_memoria)+ sizeof(t_memoria)*( list_size(aux->args) + list_size(aux->vars) );
			n++;
		}
		size_retorno += size_in_stack + 2 * sizeof(int);
		retorno = malloc(size_retorno); bzero(retorno,size_retorno);
		*devolveme = size_retorno;
		int desplazamiento = 0;

		// 4 BYTES C/U PARA: PID,PC,CANT_PAGINAS,SP,EXIT_CODE,QUANTUM,QUANTUM_SLEEP

		memcpy(retorno+desplazamiento,&pcb->PID,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&pcb->PC,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&pcb->cant_pag,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&pcb->SP,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&pcb->exit_code,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&quantum,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&quantum_sleep,sizeof(int));
		desplazamiento += sizeof(int);

		// 2 BYTES PARA ALGORITMO
		memcpy(retorno+desplazamiento,algoritmo,2*sizeof(char));
		desplazamiento += 2* sizeof(char);

		// 4 BYTES PARA TAMAÑO_SENTENCIAS_SERIALIZADAS
		memcpy(retorno +desplazamiento,&size_in_sentencias,sizeof(int));
		desplazamiento += sizeof(int);

		// SERIALIZO SENTENCIAS
		n=0;
		while(pcb->in_cod[n].offset_inicio != -1 &&pcb->in_cod[n].offset_fin != -1 ){
			memcpy(retorno+desplazamiento, &pcb->in_cod[n].offset_inicio,sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(retorno+desplazamiento, &pcb->in_cod[n].offset_fin,sizeof(int));
			desplazamiento += sizeof(int);
				n++;
		}
		memcpy(retorno+desplazamiento, &pcb->in_cod[n].offset_fin,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento, &pcb->in_cod[n].offset_fin,sizeof(int));
		desplazamiento += sizeof(int);
		// AGREGO EL INDICE_ETIQUETAS (DICCIONARIO DE ETIQUETAS SERIALIZADO)
		memcpy(retorno+desplazamiento,pcb->in_et,size_in_et);
		desplazamiento += size_in_et;

		// 4 BYTES C/U PARA : SIZE_IN_STACK, CANT_ELEMENTOS_STACK
		memcpy(retorno+desplazamiento,&size_in_stack,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(retorno+desplazamiento,&tam_stack,sizeof(int));
		desplazamiento += sizeof(int);

		n=0;
		while (n < tam_stack){
			t_stack_element* aux = list_get(pcb->in_stack,n);
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
			n_args= list_size(aux->args);n_vars = list_size(aux->vars);
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

		return retorno;
}

t_PCB* deserializarPCB_CPUKer (char* pcbserializado){
	//int sizeb_mensaje =0;
	int desplazamiento =0;
	int size_in_cod = 0; int size_in_et = 0;
	int size_in_stack = 0; int cant_stack = 0;

	t_PCB *pcb = malloc(sizeof(t_PCB));
	memcpy(&pcb->PID,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&pcb->PC,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(&pcb->cant_pag,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&pcb->SP,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&pcb->exit_code,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&size_in_cod,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	pcb->in_cod = malloc(sizeof(t_sentencia) *(size_in_cod / sizeof(t_sentencia)));

	int n=0;
	while(n < (size_in_cod / sizeof(t_sentencia))){
		t_sentencia aux;
		memcpy(&aux.offset_inicio,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&aux.offset_fin,pcbserializado+desplazamiento,sizeof(int));
		desplazamiento += sizeof(int);
		pcb->in_cod[n]= aux;
		n++;
	}
	memcpy(&size_in_et,pcbserializado+desplazamiento,sizeof(int));
	pcb->in_et = malloc(size_in_et);
	memcpy(pcb->in_et,pcbserializado+desplazamiento,size_in_et);
	desplazamiento += size_in_et;
	memcpy(&size_in_stack,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&cant_stack,pcbserializado+desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	pcb->in_stack = list_create();
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
		list_add(pcb->in_stack,aux);
		n++;
	}
	return pcb;
}
