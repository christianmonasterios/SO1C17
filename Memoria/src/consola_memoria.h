/*
 * consola_memoria.h
 *
 *  Created on: 8/7/2017
 *      Author: utnso
 */

#ifndef CONSOLA_MEMORIA_H_
#define CONSOLA_MEMORIA_H_

void hilo_consola_memoria();
int que_case_es(char *);
void display_menu_principal();
void display_menu_dump();
void limpiar_cache();
char *dump_cache();
char *dump_estructuras();
char *dump_datos(int pid);
int marcos_libres();
int calcular_paginas_de_proceso(int pid);
char *info_marco_proceso(int pid);

#endif /* CONSOLA_MEMORIA_H_ */
