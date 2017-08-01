/*
 * consolaManager.h
 *
 *  Created on: 28/6/2017
 *      Author: utnso
 */

#ifndef CONSOLAMANAGER_H_
#define CONSOLAMANAGER_H_

int get_CID();
void desconectar_consola(int socket);
void responder_solicitud_consola(int socket, char *mensaje);
void responder_peticion_prog(int socket, char *mensaje);
int buscar_consola(int socket);
void realizar_handShake_consola(int nuevo_socket);
char *armar_mensaje_memoria(char *mensaje_recibido);
void eliminar_consola(int consola_id);

#endif /* CONSOLAMANAGER_H_ */
