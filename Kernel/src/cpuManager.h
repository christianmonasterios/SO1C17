#ifndef CPUMANAGER_H_
#define CPUMANAGER_H_
#include "estructuras.h"

void realizar_handShake_cpu(int);
void agregar_lista_cpu(int);
int get_cpuId();
void actualizar_pcb();
void responder_solicitud_cpu(int socket_, char *mensaje);
t_cpu *buscar_cpu(int socket_);
char *get_offset(char *mensaje);
char *get_fd(char *mensaje);
char *get_variable(char *mensaje);
char *get_numero(char *mensaje);
void pedir_pcb_error(t_program *prg, int exit_code);
char *armar_valor(char *pid_, char *mensaje);
void eliminar_cpu(int socket_);
void recargar_quantumsleep();
void procesar_cambio_configuracion(int socket_rec);
char *sacar_nombre_archivo(char *path);
char *get_offset2(char *mensaje);
char *get_fd2(char *mensaje);

#endif /* CPUMANAGER_H_ */
