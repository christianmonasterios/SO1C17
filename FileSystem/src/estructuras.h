/*
 * estructuras.h
 *
 *  Created on: 11/6/2017
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

typedef struct{
	int tamanio;
	char ** bloques;
}t_arch;

typedef struct{
	char * path;
	int offset;
	int size;
	char * buffer;
}t_datos;

#endif /* ESTRUCTURAS_H_ */
