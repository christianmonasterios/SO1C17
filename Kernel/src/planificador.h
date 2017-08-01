/*
 * planificador.h
 *
 *  Created on: 7/5/2017
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

//Agrega a la cola un nuevo programa para ser atendido
void agregar_nueva_prog(int id_consola, int pid, char *mensaje, int sock_con);
void forzar_finalizacion(int pid, int cid, int codigo_finalizacion, int aviso);
void finalizar_proceso(int pid, int codigo_finalizacion);
void programas_listos_A_ejecutar();
void programas_nuevos_A_listos();
void bloquear_proceso(int pid, int socket_);
void desbloquear_proceso(int);
int calcular_pag(char *mensaje);
void finalizar_quantum(int pid);
void actualizar_grado_multiprogramacion();

#endif /* PLANIFICADOR_H_ */
