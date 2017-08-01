/*
 * memoria.h
 *
 *  Created on: 7/7/2017
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_
#include "estructuras.h"
#include <commons/collections/list.h>

void handshakearMemory();
t_bloque *find_first_fit(t_list *hs, int t_sol);
void reservar_memoria_din(t_program *program, int size_solicitado, int so_cpu);
void inicializar_pagina_dinamica(t_program *prog, int size_sol);
int ubicar_bloque(t_pagina *pagina, int tam_sol, t_program *program, int so_cpu);
void pedir_pagina_dinamica(t_program *prog);
void compactar(t_pagina *pagina);
void _free_bloque(t_bloque *bloque);
int pedir_pagina();
void liberar_bloque(t_program *prog, char *offset_bloque, int socket_);
void liberar_pagina(t_pagina *pagina);
int almacenar_bytes(int pid, int numpag, int offset, int tam, char *buffer);
void liberar_proceso_pagina(int pid);
void *pedir_bloque_libre(t_pagina *pagina, int pid, int tam_sol, int *inicio_bloque);

#endif /* MEMORIA_H_ */
