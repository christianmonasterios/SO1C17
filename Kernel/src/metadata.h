/*
 * metadata.h
 *
 *  Created on: 1/6/2017
 *      Author: utnso
 */

#ifndef METADATA_H_
#define METADATA_H_
#include "estructuras.h"

t_sentencia* armarIndiceCodigo (char *codigoPrograma);
char* armarIndiceEtiquetas(char *codigoPrograma);
t_list* armarIndiceStack(char *codigoPrograma);
char* serializarPCB_KerCPU(t_PCB* pcb,char *,int ,int ,int *);
t_PCB* deserializarPCB_CPUKer (char* );

#endif /* METADATA_H_ */
