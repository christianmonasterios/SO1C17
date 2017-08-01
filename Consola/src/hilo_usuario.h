/*
 * hilo_usuario.h
 *
 *  Created on: 6/5/2017
 *      Author: utnso
 */

#ifndef HILO_USUARIO_H_
#define HILO_USUARIO_H_

void hilousuario ();
char *leer_archivo(char*);
//void mostrar(void *algo);
void cerrar (void * pid);
void iniciar_programa(char * ruta, int socket_);
void verificar_finalizar(char * identi, int socket_);
void finalizar_noiniciados(char * pid, int socket_);
void finalizar_programa(char * pid, int socket_);
void desconectar_consola();
void cerrar_programas(char* key, void* data);
void tiempofinal_impresiones(char* pid);
void mostrar_pids(char* key, void* data);

#endif /* HILO_USUARIO_H_ */
