/*
 * hash.h
 *
 *  Created on: 7/7/2017
 *      Author: utnso
 */

#ifndef HASH_H_
#define HASH_H_
#include <stdbool.h>
#include <commons/collections/dictionary.h>

/* HASH */

 // asignar esta variable con el ultimo frame asignado oficialmente DE UNA COLISION

void inicializar_array_colisiones();
// -> PROCESO DE ASIGNACION DE MARCOS
int hash(int pid,int pag);
bool marco_ocupado(int marco); // true (1) ocupado | false(0) libre
//int reasignar_colision(int marco,int pid,int pag); // si el marco anterior estaba ocupado,le asigna un nuevo marco a una colision, despues hay que asentar la colision
int reasignar_colision();
void asentar_colision(int marcoFinal,int marcoGeneradorColision,int pid, int pagina); // deja constancia de la colision y como se soluciono
// -> PROCESO DE BUSQUEDA DE MARCOS
// primero con pid y pag aplicar funcion marco=HASH(PID,PAG)
bool es_marco_correcto(int pid, int pagina,int marco);
// si no es el marco correcto es porque hubo una colision al momento de asignacion, asi que busco el marco correcto con buscar_marco_colision
int buscar_marco_colision(int pid,int pagina,int marcoincorrecto); // me devuelve el marco correcto donde se alojo la colision

int eliminar_colision(int pid,int pagina, int marcoincorrecto); //elimina del diccionario asociado, devuelve el marco real de la tabla de paginas para actualizar estado manualmente



#endif /* HASH_H_ */
