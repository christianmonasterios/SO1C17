#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdbool.h>
#include "estructuras.h"

int abrir_archivo(char *path, char* flag, t_program *prog);
t_TAG *buscar_archivo_TAG(char *p_sol);
void abrir_crear(char *mensaje, t_program *prog, int socket_cpu);
void pedido_lectura(t_program *prog, int fd, int offs, int size, char *path, int socket_cpu);
char *info_lectura(char *path,int offs,int size);
t_TAP *buscar_archivo_TAP(t_list *tap, int fd);
char *get_path(int fd);
t_TAG *buscar_archivo_TAG_fd(int fd);
void mover_puntero(int socket_prog, int offset, int fd, t_program *prog);
bool existe_archivo(t_list *tap, int fd);
void destruir_file(t_TAP *ap);
void destruir_file_TAG(t_TAG *tg);
void cerrar_file(int pid, t_list *tap, int fd, int socket_);
char *get_path_msg(char *mensaje, int *payload1);
char *get_info(char *mensaje, int payload1, int tam_info);
void abrir_crear(char *mensaje, t_program *prog, int socket_cpu);
void escribir_archivo(int largo,int offset, char *info, char *flags, char *path, int socket_cpu, t_program *pr);
int chequear_respuesta(int socket_cpu, char *path, char *flag, t_program *prog);
int crear_archivo(int socket_cpu, char *path, char *flag, t_program *prog);
char *armar_info_mensaje(int largo,char *info, char* path, char *o,int *l);
void borrar_archivo(int pid, t_list *tap, int fd, int socket_);

#endif /* FILESYSTEM_H_ */
