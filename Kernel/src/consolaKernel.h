/*
 * consolaKernel.h
 *
 *  Created on: 25/5/2017
 *      Author: utnso
 */

#ifndef CONSOLAKERNEL_H_
#define CONSOLAKERNEL_H_
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

void generar_listados(int lista);
void leer_consola();
void imprimir_menu();
void mostrar_cola(t_queue *, char *);
void mostrar_listas(t_list *, char *);
void obtener_informacion(int pid);
void imprimir_tabla_archivos();
char *devolver_descripcion_error(int codigo);
int existe_pid(int pid);
void mostrar_cola_listos(t_queue *cola, char *procesos);
void imprimir_info();

#endif /* CONSOLAKERNEL_H_ */
