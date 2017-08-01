/*
 * funcionesParser.h
 *
 *  Created on: 31/5/2017
 *      Author: utnso
 */

#ifndef SRC_FUNCIONESPARSER_H_
#define SRC_FUNCIONESPARSER_H_
#include <parser/parser.h>
#include <parser/metadata_program.h>
AnSISOP_funciones funcionesTodaviaSirve;
AnSISOP_kernel funcionesKernelTodaviaSirve;

t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable,t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void finalizar(void);
void retornar(t_valor_variable retorno);
void wait(t_nombre_semaforo identificador_semaforo);
void signale(t_nombre_semaforo identificador_semaforo);
void escribir (t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio);
void moverCursor (t_descriptor_archivo descriptor_archivo, t_valor_variable posicion);
void liberar (t_puntero puntero);
t_puntero reservar (t_valor_variable espacio);
t_descriptor_archivo abrir (t_direccion_archivo direccion, t_banderas flags);
void leer (t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio);
void cerrar (t_descriptor_archivo descriptor_archivo);
void borrar (t_descriptor_archivo direccion);


#endif /* SRC_FUNCIONESPARSER_H_ */
