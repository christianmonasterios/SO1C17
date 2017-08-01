/*
 * mensaje.h
 *
 *  Created on: 23/4/2017
 *      Author: utnso
 */

#ifndef MENSAJE_H_
#define MENSAJE_H_

//Arma el texto de envio para mensajes
char *armar_mensaje(char *identificador, char *mensaje);
char *armar_mensaje_pcb(char *identificador, char *mensaje,int sizepcb);

//Devuelve el header del mensaje
char *get_header(char *mensaje);

//Devuelve el codigo del mensaje
char * get_codigo(char *mensaje);

//Obtiene el mensaje
char * get_mensaje(char *mensaje);
char *get_mensaje_pcb(char *mensaje);

//Compara un header contra el header un mensaje
int comparar_header(char *identificador, char *header);

char *get_mensaje_escritura_info(char *mensaje,int *lar);

char *get_mensaje_escritura_fd(char *mensaje);

#endif /* MENSAJE_H_ */
