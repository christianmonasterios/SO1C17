/*
 * mensajes.c
 *
 *  Created on: 4/7/2017
 *      Author: utnso
 */

#include "mensajes.h"
#include <commons/string.h>
char *mensaje_escibir_memoria(int fpid,t_puntero direccion_variable,int cant_pag,int valor,int *size){

	char * mensaje;
	char * pid; char * pagina; char *offset; char *tam;
	char * aux_ceros;
	int desplazamiento=0;
	pid = string_itoa(fpid);
	pagina = string_itoa(calcular_pagina(direccion_variable,cant_pag));
	offset = string_itoa(calcular_offset_respecto_pagina(direccion_variable));
	/*char *str_valor = string_itoa(valor);
	if(strlen(str_valor)>4){
		char *aux_2 = string_repeat('0',4-strlen(str_valor));
		tam = string_from_format("%s%d",aux_2,strlen(str_valor));
		free(aux_2);
	}else{
		tam = strdup("0004");
	}*/
	tam = strdup("0004");

	/*if(strlen(str_valor)>4){
	mensaje = malloc(19+ strlen(str_valor));
	}else{
		mensaje = malloc(19+ 4);
	}
	*/
	mensaje = malloc(19+4);
	// COD
	memcpy(mensaje+desplazamiento,"P08",3);
	desplazamiento += 3;
	// PID
	aux_ceros = string_repeat('0',4-strlen(pid));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(pid));
	free(aux_ceros);
	desplazamiento += 4-strlen(pid);
	memcpy(mensaje+desplazamiento,pid,strlen(pid));
	desplazamiento += strlen(pid);
	// PAGINA
	aux_ceros = string_repeat('0',4-strlen(pagina));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(pagina));
	free(aux_ceros);
	desplazamiento += 4-strlen(pagina);
	memcpy(mensaje+desplazamiento,pagina,strlen(pagina));
	desplazamiento += strlen(pagina);
	// OFFSET
	aux_ceros = string_repeat('0',4-strlen(offset));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(offset));
	free(aux_ceros);
	desplazamiento += 4-strlen(offset);
	memcpy(mensaje+desplazamiento,offset,strlen(offset));
	desplazamiento += strlen(offset);
	// TAMAÑO
	memcpy(mensaje+desplazamiento,tam,4);
	desplazamiento += 4;
	// VALOR
	/*if(strlen(str_valor)>4){
		memcpy(mensaje+desplazamiento,str_valor,strlen(str_valor));
		desplazamiento += strlen(str_valor);
	}else{
		aux_ceros = string_repeat('0',4-strlen(str_valor));
		memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(str_valor));
		free(aux_ceros);
		desplazamiento += 4-strlen(str_valor);
		memcpy(mensaje+desplazamiento,str_valor,strlen(str_valor));
		desplazamiento += strlen(str_valor);
	}*/
	memcpy(mensaje+desplazamiento,&valor,4);
	desplazamiento += 4;
	*size = desplazamiento;

	free(pid); free(pagina); free(offset); free(tam); //free(str_valor);

	return mensaje;
}
char *mensaje_semaforo(char * cod,char * semaforo,int *size){
	char *mensaje= malloc(13+strlen(semaforo));
	char *strlen_sem = string_itoa(strlen(semaforo));
	char *aux_ceros = string_repeat('0',10-strlen(strlen_sem));

	int desplazamiento = 0;
	memcpy(mensaje + desplazamiento,cod,3);
	desplazamiento += 3;
	memcpy(mensaje+desplazamiento,aux_ceros,10-strlen(strlen_sem));
	desplazamiento += 10-strlen(strlen_sem);
	memcpy(mensaje+desplazamiento,strlen_sem,strlen(strlen_sem));
	desplazamiento += strlen(strlen_sem);
	memcpy(mensaje+desplazamiento,semaforo,strlen(semaforo));
	desplazamiento += strlen(semaforo);

	*size = desplazamiento;
	free(strlen_sem);
	free(aux_ceros);
	return mensaje;
}
char *mensaje_leer_memoria(int fpid,t_puntero direccion_variable, int cant_pag,int size_lectura,int *size){

	char *mensaje = malloc(19);
	char *pid;
	char *pagina;
	char *offset;
	char *str_size_lectura = string_itoa(size_lectura);
	char * aux_ceros;
	int desplazamiento = 0;
	pid = string_itoa(fpid);
	pagina = string_itoa(calcular_pagina(direccion_variable,cant_pag));
	offset = string_itoa(calcular_offset_respecto_pagina(direccion_variable));

	aux_ceros = string_repeat('0',4-strlen(str_size_lectura));
	char *tam = string_from_format("%s%s",aux_ceros,str_size_lectura);
	free(aux_ceros);
	// PID
	memcpy(mensaje+desplazamiento,"P01",3);
	desplazamiento += 3;
	aux_ceros = string_repeat('0',4-strlen(pid));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(pid));
	free(aux_ceros);
	desplazamiento += 4-strlen(pid);
	memcpy(mensaje+desplazamiento,pid,strlen(pid));
	desplazamiento += strlen(pid);
	// PAGINA
	aux_ceros = string_repeat('0',4-strlen(pagina));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(pagina));
	free(aux_ceros);
	desplazamiento += 4-strlen(pagina);
	memcpy(mensaje+desplazamiento,pagina,strlen(pagina));
	desplazamiento += strlen(pagina);
	// OFFSET
	aux_ceros = string_repeat('0',4-strlen(offset));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(offset));
	free(aux_ceros);
	desplazamiento += 4-strlen(offset);
	memcpy(mensaje+desplazamiento,offset,strlen(offset));
	desplazamiento += strlen(offset);
	// TAMAÑO
	memcpy(mensaje+desplazamiento,tam,strlen(tam));
	desplazamiento += strlen(tam);

	free(pid);
	free(pagina);
	free(offset);
	free(tam);
	free(str_size_lectura);

	*size = desplazamiento;

	return mensaje;
}
char *mensaje_pcb(char *identificador, char *mensaje,int sizepcb)
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
char *mensaje_escribir_kernel(int fd,void *informacion,int tamanio,int *size){

	char * mensaje;
	int desplazamiento = 0;
	char *aux_ceros;
	char *str_valor;
	char *cod;
	int tam_alloc;
	// COD
	if(fd == 1){
		cod=strdup("P11");
		//str_valor = string_itoa(strlen(informacion));
		str_valor = string_itoa(tamanio);
		tam_alloc = 13+tamanio+1;
	}else{
		cod=strdup("P05");
		//str_valor = string_itoa(strlen(informacion)+4);
		str_valor = string_itoa(tamanio+4);
		tam_alloc = 13+4+tamanio+1;
	};
	mensaje = malloc(tam_alloc);
	memset(mensaje,'\0',tam_alloc);
	memcpy(mensaje+desplazamiento,cod,3);
	desplazamiento += 3;
	aux_ceros = string_repeat('0',10-strlen(str_valor));
	memcpy(mensaje+desplazamiento,aux_ceros,10-strlen(str_valor));
	free(aux_ceros);
	desplazamiento += 10-strlen(str_valor);
	memcpy(mensaje+desplazamiento,str_valor,strlen(str_valor));
	desplazamiento += strlen(str_valor);
	if(fd != 1){
		char *sfd = string_itoa(fd);
		aux_ceros= string_repeat('0',4-strlen(sfd));
		memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(sfd));
		desplazamiento += 4-strlen(sfd);
		free(aux_ceros);
		memcpy(mensaje+desplazamiento,sfd,strlen(sfd));
		desplazamiento += strlen(sfd);
		free(sfd);
	}

	// VARIABLE
	memcpy(mensaje+desplazamiento,informacion,tamanio);
	desplazamiento += tamanio;

	*size= desplazamiento;
	free(str_valor);free(cod);

	return mensaje;
}
char *mensaje_variable_kernel(int codigo,char *variable,int valor,int *size){

	char *mensaje;
	char *str_size_variable;
	char *aux_ceros;
	char *cod;
	int final;int desplazamiento = 0;

	if(codigo == 10){
		cod = strdup("P10");
		final = 13+strlen(variable)+4;
		str_size_variable = string_itoa(strlen(variable)+4);

	}else{
		cod =  strdup("P09");
		final = 13+strlen(variable);
		str_size_variable  = string_itoa(strlen(variable));

	}
	aux_ceros = string_repeat('0',10-strlen(str_size_variable));


	mensaje= malloc(final);
	memcpy(mensaje + desplazamiento,cod,3);
	desplazamiento += 3;
	memcpy(mensaje + desplazamiento,aux_ceros,10-strlen(str_size_variable));
	desplazamiento += 10-strlen(str_size_variable);
	memcpy(mensaje + desplazamiento,str_size_variable,strlen(str_size_variable));
	desplazamiento += strlen(str_size_variable);
	memcpy(mensaje + desplazamiento,variable,strlen(variable));
	desplazamiento += strlen(variable);

	free(aux_ceros);
	free(str_size_variable);
	free(cod);

	if(codigo == 10){
		char *str_valor = string_itoa(valor);
		aux_ceros = string_repeat('0',4-strlen(str_valor));
		memcpy(mensaje + desplazamiento,aux_ceros,4-strlen(str_valor));
		desplazamiento += 4-strlen(str_valor);
		memcpy(mensaje + desplazamiento,str_valor,strlen(str_valor));
		desplazamiento += strlen(str_valor);
		free(str_valor);
		free(aux_ceros);
	}

	*size = desplazamiento;
	return mensaje;
}
char *mensaje_heap(char *cod,int valor, int *size){


	char *str_valor = string_itoa(valor);
	char *str_largo_valor = string_itoa(strlen(str_valor));
	char *aux_ceros = string_repeat('0',10-strlen(str_largo_valor));
	char *mensaje = malloc(13+strlen(str_valor));

	int desplazamiento=0;
	memcpy(mensaje+desplazamiento,cod,3);
	desplazamiento += 3;
	memcpy(mensaje+desplazamiento,aux_ceros,10-strlen(str_largo_valor));
	desplazamiento += 10-strlen(str_largo_valor);
	memcpy(mensaje+desplazamiento,str_largo_valor,strlen(str_largo_valor));
	desplazamiento += strlen(str_largo_valor);
	memcpy(mensaje+desplazamiento,str_valor,strlen(str_valor));
	desplazamiento += strlen(str_valor);

	free(aux_ceros);
	free(str_valor);
	free(str_largo_valor);

	*size = desplazamiento;

	return mensaje;
}
char *mensaje_borrar_cerrar(int cod,int fd,int *size){

	int desplazamiento = 0;
	char *str_fd = string_itoa(fd);
	char *mensaje = malloc(13+strlen(str_fd));
	char *size_mensaje = string_itoa(strlen(str_fd));
	char *aux_ceros = string_repeat('0',10-strlen(size_mensaje));
	char *codigo;

	if(cod == 6 ){
		codigo = strdup("P06");
	}else if(cod == 7){
		codigo = strdup("P07");
	}

	memcpy(mensaje+desplazamiento,codigo,3);
	desplazamiento += 3;
	memcpy(mensaje+desplazamiento,aux_ceros,10-strlen(size_mensaje));
	desplazamiento += 10-strlen(size_mensaje);
	memcpy(mensaje+desplazamiento,size_mensaje,strlen(size_mensaje));
	desplazamiento += strlen(size_mensaje);
	memcpy(mensaje+desplazamiento,str_fd,strlen(str_fd));
	desplazamiento += strlen(str_fd);

	*size = desplazamiento;

	free(str_fd);
	free(size_mensaje);
	free(aux_ceros);
	free(codigo);

	return mensaje;
}
char *mensaje_moverCursor(int fd,int posicion,int * size){

	int desplazamiento = 0;
	char *str_fd = string_itoa(fd);
	char *mensaje = malloc(13+strlen(str_fd)+4);
	char *size_mensaje = string_itoa(strlen(str_fd)+4);
	char *aux_ceros = string_repeat('0',10-strlen(size_mensaje));
	char *str_posicion = string_itoa(posicion);
	char *codigo = strdup("P03");

	memcpy(mensaje+desplazamiento,codigo,3);
	desplazamiento += 3;
	memcpy(mensaje+desplazamiento,aux_ceros,10-strlen(size_mensaje));
	desplazamiento += 10-strlen(size_mensaje);
	memcpy(mensaje+desplazamiento,size_mensaje,strlen(size_mensaje));
	desplazamiento += strlen(size_mensaje);
	memcpy(mensaje+desplazamiento,str_fd,strlen(str_fd));
	desplazamiento += strlen(str_fd);

	free(aux_ceros);
	aux_ceros = string_repeat('0',4-strlen(str_posicion));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(str_posicion));
	desplazamiento += 4-strlen(str_posicion);
	memcpy(mensaje+desplazamiento,str_posicion,strlen(str_posicion));
	desplazamiento += strlen(str_posicion);

	*size = desplazamiento;

	free(str_fd);
	free(size_mensaje);
	free(aux_ceros);
	free(codigo);
	free(str_posicion);

	return mensaje;
}

char *mensaje_abrir(char *direccion,t_banderas flags,int *size){

	int desplazamiento = 0;
	char *mensaje;
	char *banderas = strdup("");
	char *str_direccion = string_itoa(strlen(direccion));
	int cantidad_banderas=0;
	char *c = strdup("c");char *r = strdup("r");char *w = strdup("w");

	if(flags.creacion == 1){
		string_append(&banderas,c);
		cantidad_banderas ++;
	}
	if(flags.escritura == 1){
		string_append(&banderas,w);
		cantidad_banderas ++;
	}
	if(flags.lectura == 1){
		string_append(&banderas,r);
		cantidad_banderas ++;
	}
	char *size_mensaje = string_itoa(strlen(direccion)+4+1+cantidad_banderas);
	char *aux_ceros = string_repeat('0',10-strlen(size_mensaje));

	mensaje = malloc(13+strlen(direccion)+4+1+cantidad_banderas);
	char *str_banderas = string_itoa(cantidad_banderas);

	memcpy(mensaje+desplazamiento,"P02",3);
	desplazamiento += 3;
	memcpy(mensaje+desplazamiento,aux_ceros,10-strlen(size_mensaje));
	desplazamiento += 10-strlen(size_mensaje);
	memcpy(mensaje+desplazamiento,size_mensaje,strlen(size_mensaje));
	desplazamiento += strlen(size_mensaje);
	free(aux_ceros);
	aux_ceros = string_repeat('0',4-strlen(str_direccion));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(str_direccion));
	desplazamiento += 4-strlen(str_direccion);
	memcpy(mensaje+desplazamiento,str_direccion,strlen(str_direccion));
	desplazamiento += strlen(str_direccion);
	memcpy(mensaje+desplazamiento,direccion,strlen(direccion));
	desplazamiento += strlen(direccion);
	memcpy(mensaje+desplazamiento,str_banderas,1);
	desplazamiento ++;
	memcpy(mensaje+desplazamiento,banderas,cantidad_banderas);
	desplazamiento += cantidad_banderas;

	*size = desplazamiento;

	free(banderas);
	free(size_mensaje);
	free(aux_ceros);
	free(c);
	free(r);
	free(w);
	free(str_banderas);
	free(str_direccion);

	return mensaje;
}

char *mensaje_leer_kernel(int fd,int tamanio,int *size){

	int desplazamiento = 0;
	char *cod = strdup("P04");
	char *str_fd = string_itoa(fd);
	char *str_tamanio =  string_itoa(tamanio);
	char *str_long_mensaje = string_itoa(strlen(str_tamanio)+4);
	char *aux_ceros = string_repeat('0',10-strlen(str_long_mensaje));
	char *mensaje = malloc(13+strlen(str_tamanio)+4);

	memcpy(mensaje+desplazamiento,cod,3);
	desplazamiento += 3;
	memcpy(mensaje+desplazamiento,aux_ceros,10-strlen(str_long_mensaje));
	desplazamiento += 10-strlen(str_long_mensaje);
	memcpy(mensaje+desplazamiento,str_long_mensaje,strlen(str_long_mensaje));
	desplazamiento += strlen(str_long_mensaje);
	free(aux_ceros);
	aux_ceros = string_repeat('0',4-strlen(str_fd));
	memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(str_fd));
	desplazamiento += 4-strlen(str_fd);
	memcpy(mensaje+desplazamiento,str_fd,strlen(str_fd));
	desplazamiento += strlen(str_fd);
	memcpy(mensaje+desplazamiento,str_tamanio,strlen(str_tamanio));
	desplazamiento += strlen(str_tamanio);

	*size = desplazamiento;

	free(cod);
	free(str_fd);
	free(str_tamanio);
	free(str_long_mensaje);
	free(aux_ceros);

	return mensaje;
}

char *mensaje_escibir_noint_memoria(int fpid,t_puntero direccion_variable,int cant_pag,int largo,void *valor,int *size){

		char * mensaje = malloc(19+largo);
		char * pid; char * pagina; char *offset; char *tam;
		char * aux_ceros;
		int desplazamiento=0;
		pid = string_itoa(fpid);
		pagina = string_itoa(calcular_pagina(direccion_variable,cant_pag));
		offset = string_itoa(calcular_offset_respecto_pagina(direccion_variable));
		tam = string_itoa(largo);
		// COD
		memcpy(mensaje+desplazamiento,"P08",3);
		desplazamiento += 3;
		// PID
		aux_ceros = string_repeat('0',4-strlen(pid));
		memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(pid));
		free(aux_ceros);
		desplazamiento += 4-strlen(pid);
		memcpy(mensaje+desplazamiento,pid,strlen(pid));
		desplazamiento += strlen(pid);
		// PAGINA
		aux_ceros = string_repeat('0',4-strlen(pagina));
		memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(pagina));
		free(aux_ceros);
		desplazamiento += 4-strlen(pagina);
		memcpy(mensaje+desplazamiento,pagina,strlen(pagina));
		desplazamiento += strlen(pagina);
		// OFFSET
		aux_ceros = string_repeat('0',4-strlen(offset));
		memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(offset));
		free(aux_ceros);
		desplazamiento += 4-strlen(offset);
		memcpy(mensaje+desplazamiento,offset,strlen(offset));
		desplazamiento += strlen(offset);
		// TAMAÑO
		aux_ceros = string_repeat('0',4-strlen(tam));
		memcpy(mensaje+desplazamiento,aux_ceros,4-strlen(tam));
		free(aux_ceros);
		desplazamiento += 4-strlen(tam);
		memcpy(mensaje+desplazamiento,tam,strlen(tam));
		desplazamiento += strlen(tam);
		// VALOR

		memcpy(mensaje+desplazamiento,valor,largo);
		desplazamiento += largo;
		*size = desplazamiento;

		free(pid); free(pagina); free(offset); free(tam);
		return mensaje;
}
