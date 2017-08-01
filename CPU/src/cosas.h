/*
 * cosas.h
 *
 *  Created on: 20/5/2017
 *      Author: utnso
 */

#ifndef SRC_COSAS_H_
#define SRC_COSAS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include "estructuras.h"
#include <parser/metadata_program.h>

t_sentencia* armarIndiceCodigo (char *codigoPrograma);
char* armarIndiceEtiquetas(char *codigoPrograma);
t_list* armarIndiceStack(char *codigoPrograma);
char* serializarPCB_KerCPU (t_PCB*,char *,int,int,int *);
char* serializarPCB_CPUKer2 (t_PCB_CPU*,int *);
t_PCB* deserializarPCB_CPUKer (char* );
t_PCB_CPU* deserializarPCB_KerCPU2 (char* );
#endif /* SRC_COSAS_H_ */
