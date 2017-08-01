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
void escribir_log(char *mensaje,int cod);

void liberar_log();

#endif /* SRC_LOG_H_ */
