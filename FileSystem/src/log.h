/*
 * log.h
 *
 *  Created on: 29/11/2016
 *      Author: utnso
 */

#ifndef SRC_LOG_H_
#define SRC_LOG_H_

//Funcion para crear archivo de log
void crear_archivo_log(char *file);

//Funcion que escribe en archivo de log
void escribir_log(char *mensaje);

//Funcion que escribe un ERROR en archivo de log
void escribir_error_log(char *mensaje);

//Recibe un texto y un numerico, muy util!
void escribir_log_con_numero(char *mensaje, int un_numero);

//Recibe y genera log recibiendo dos textos
void escribir_log_compuesto(char *mensaje, char *otro_mensaje);

//Recibe y genera log de ERROR recibiendo dos textos
void escribir_log_error_compuesto(char *mensaje, char *otro_mensaje);

void liberar_log();

#endif /* SRC_LOG_H_ */
