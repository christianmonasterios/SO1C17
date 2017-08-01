/*
 * funciones.h
 *
 *  Created on: 4/6/2017
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

void validar_archivo(char *mensaje);
void crear_archivo(char *mensaje);
void borrar_archivo(char *mensaje);
void obtener_datos(char *path, int offset, int size);
void guardar_datos(char *path, int offset, int size, char *buffer);


#endif /* FUNCIONES_H_ */
