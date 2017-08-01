/*
 * escuchar_mensaje.h
 *
 *  Created on: 28/5/2017
 *      Author: utnso
 */

#ifndef ESCUCHAR_MENSAJE_H_
#define ESCUCHAR_MENSAJE_H_

void escuchar_mensaje();
void verificacion_finalizar(char * pid);
void finalizar(char *pid, int socket_);
void finalizar_no_iniciados(char * pid, int socket_);
void senial();

#endif /* ESCUCHAR_MENSAJE_H_ */
