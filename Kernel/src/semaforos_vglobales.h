/*
 * semaforos.h
 *
 *  Created on: 27/5/2017
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include "estructuras.h"

void inicializar_sems();
void sem_signal(t_program *prog, char *sema, int socket_, int free_all);
void sem_wait_(t_program *proceso, char *sema, int socket_);
void inicializar_vglobales();
int lock_vglobal(t_vglobal *vg, int prog);
void unlock_vglobal();
void set_vglobal(char *vglobal, int num, t_program *prog, int socket_);
void get_vglobal(char *vglobal, t_program *prog, int socket_);

#endif /* SEMAFOROS_H_ */
