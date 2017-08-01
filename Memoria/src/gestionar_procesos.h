/*
 * gestionar_procesos.h
 *
 *  Created on: 14/7/2017
 *      Author: utnso
 */

#ifndef GESTIONAR_PROCESOS_H_
#define GESTIONAR_PROCESOS_H_

#include "estructuras.h"

void crear_proceso(int pid, int pag);
int buscar_proceso(int pid);
void eliminar_proceso(int pid);
void destructor(t_proceso *self);
void destruir_procesos();
void actualizar_paginas(int pid,int asumar);
int ultimoNumeroPagina(int pid);
void actualizar_maxnro_pagina(int pid,int nro);
int cantidadPaginas(int pid);


#endif /* GESTIONAR_PROCESOS_H_ */
