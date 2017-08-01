#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "fileSystem.h"
#include "estructuras.h"
#include "configuracion.h"
#include "socket.h"
#include "manejo_errores.h"
#include "log.h"
#include "mensaje.h"
#include "planificador.h"
#include "manejo_conexiones.h"

extern t_list *global_fd;
extern t_configuracion *config;


void abrir_crear(char *mensaje, t_program *prog, int socket_cpu)
{
	char *payload_path = string_substring(mensaje,13,4);
	int payload_path_int = atoi(payload_path);

	char *payload_flag = string_substring(mensaje,17+payload_path_int,1);
	int payload_flag_int = atoi(payload_flag);

	char *path = string_substring(mensaje,17,payload_path_int);
	char *flag = string_substring(mensaje,17+payload_path_int+1,payload_flag_int);

	free(payload_path);
	free(payload_flag);

	int no_existe;

	char *men = armar_mensaje("K11", path);
	int controlador;
	enviar(config->cliente_fs, men, &controlador);
	free(men);

	no_existe = chequear_respuesta(socket_cpu, path, flag, prog);

	if(string_contains(flag,"c") && no_existe == 1)
	{
		int chq = crear_archivo(socket_cpu, path, flag, prog);
		if (chq == 0)
		{
			int fd = abrir_archivo(path, flag, prog);
			char *fd_char = string_itoa(fd);
			char *mensaje = armar_mensaje("K16", fd_char);
			enviar(socket_cpu, mensaje, &controlador);
			free(mensaje);
			free(fd_char);
		}
		else
		{
			forzar_finalizacion(prog->PID, 0, 13, 1);
		}
	}
	else if (no_existe == 0)
	{
		int fd = abrir_archivo(path, flag, prog);
		char *fd_char = string_itoa(fd);
		char *mensaje = armar_mensaje("K16", fd_char);
		enviar(socket_cpu, mensaje, &controlador);
		free(mensaje);
		free(fd_char);
	}
	else if(no_existe == 1)
	{
		char *fd_char = string_itoa(-2);
		char *me = armar_mensaje("K16", fd_char);
		enviar(socket_cpu, me, &controlador);
		free(me);
		free(fd_char);
	}
	//free(path);
	//free(flag);
}

int abrir_archivo(char *path, char* flag, t_program *prog)
{
	t_TAG *ag = buscar_archivo_TAG(path);

	//agrega a la tabla de archivos globales
	if(ag == NULL)
	{
		ag = malloc(sizeof(t_TAG));
		ag->open_ = 1;
		ag->FD = list_size(global_fd) + 3;
		ag->path = strdup(path);

		list_add(global_fd, ag);
	}else ag->open_ ++;

	//agrega a la tabla de archivos del proceso
	t_TAP *ar_p = malloc(sizeof(t_TAP)); //armar para la función para los free :)
	ar_p->flag = strdup(flag);
	ar_p->GFD = ag->FD;
	ar_p->FD = list_size(prog->TAP) + 3;
	ar_p->FD = ag->FD;

	list_add(prog->TAP, ar_p);
	return ar_p->FD;
}

t_TAG *buscar_archivo_TAG(char *path)
{
	bool _archivo_solicitado(t_TAG *ag)
	{
		return strcmp(ag->path, path);
	}

	t_TAG *ag_encontrado = list_find(global_fd, (void *)_archivo_solicitado);
	return ag_encontrado;
}

int crear_archivo(int socket_cpu, char *path, char *flag, t_program *prog)
{
	int controlador;
	char *mensaje;

	mensaje = armar_mensaje("K12",path);
	enviar(config->cliente_fs, mensaje, &controlador);
	free(mensaje);

	return chequear_respuesta(socket_cpu, path, flag, prog);
}

void pedido_lectura(t_program *prog, int fd, int offs, int size, char *path, int socket_cpu)
{
	t_TAP *ap = malloc(sizeof(t_TAP));
	ap = buscar_archivo_TAP(prog->TAP, fd);

	if (ap != NULL)
	{
		if(string_contains(ap->flag,"r"))
		{
			int controlador;
			char * info = info_lectura(path,offs,size);
			char *mensaje = armar_mensaje("K14", info);
			enviar(config->cliente_fs, mensaje, &controlador);
			free(mensaje);

			char *mensaje_recibido = recibir(config->cliente_fs, &controlador);
			char *header = get_header(mensaje_recibido);

			if(comparar_header(header,"F"))
			{
				char *codig = get_codigo(mensaje_recibido);
				int cod = atoi(codig);
				if(cod == 4)
				{
					char *mensaje_leido = get_mensaje(mensaje_recibido);
					char *m_envi = armar_mensaje("K88", mensaje_leido);
					enviar(socket_cpu, m_envi, &controlador);
					free(mensaje_leido);
					free(m_envi);
				}
				free(codig);
			}
			free(header);
			free(mensaje_recibido);
			free(info);

		}else
			forzar_finalizacion(prog->PID, 0, 3, 1);/* eliminar programa, pedido de lectura sin permiso*/
	}else
		forzar_finalizacion(prog->PID, 0, 2, 1);//eliminar programa por querer leer un arch no abierto
}

char *info_lectura(char *path,int offs,int s)
{
	char * mensaje = strdup("");

	char * off = string_itoa(offs);
	char * size_buffer = string_itoa(s);

	char *size_path = string_itoa(string_length(path));
	int size_payload2 = string_length(size_path);
	char *spath = string_repeat('0', 4 - size_payload2);

	char *size_offset = string_itoa(string_length(off));
	int payload = string_length(off);
	char *soffset = string_repeat('0', 4 - payload);

	char *size_ = string_itoa(string_length(size_buffer));
	int size = string_length(size_);
	char *osize = string_repeat('0', 4 - size);


	string_append(&mensaje, spath);
	string_append(&mensaje, size_path);
	string_append(&mensaje, path);

	string_append(&mensaje, soffset);
	string_append(&mensaje, size_offset);
	string_append(&mensaje, off);

	string_append(&mensaje,osize);
	string_append(&mensaje,size_);
	string_append(&mensaje,size_buffer);

	free(off);
	free(size_buffer);
	free(size_path);
	free(spath);
	free(size_offset);
	free(soffset);
	free(size_);
	free(osize);
	return mensaje;
}

t_TAP *buscar_archivo_TAP(t_list *tap, int fd)
{
	bool _archivo_TAP(t_TAP *ap)
	{
		return (ap->FD == fd);
	}

	t_TAP *ap_b = list_find(tap, (void *)_archivo_TAP);
	return ap_b;
}

char *get_path(int fd)
{
	t_TAG *ag = buscar_archivo_TAG_fd(fd);

	return ag->path;
}

t_TAG *buscar_archivo_TAG_fd(int fd)
{
	bool _archivo_fd(t_TAG *ag)
	{
		return ag->FD == fd;
	}

	t_TAG *ag_aux = list_find(global_fd, (void *)_archivo_fd);
	return ag_aux;
}

void mover_puntero(int socket_cpu, int offset, int fd, t_program *prog)
{
	int control;
	t_TAP *tap = buscar_archivo_TAP(prog->TAP, fd);

	if(tap != NULL)
	{
		enviar(socket_cpu, "OK000000000000000", &control);
		char *mensaje = recibir(socket_cpu, &control);
		char *info = get_mensaje(mensaje);

		char *path = get_path(tap->GFD);
		char *header = get_header(mensaje);

		//enviar(socket_cpu, "OK000000000000000", &control);

		if (comparar_header("P",header))
		{
			char *cod = get_codigo(mensaje);
			int codigo = atoi(cod);
			switch (codigo)
			{
				case 5 : ;//pedido de escritura
					/*char *info = strdup("");
					info = get_mensaje(mensaje);*/
					t_TAP *arch = buscar_archivo_TAP(prog->TAP, fd);
					if (arch == NULL)
					{
						forzar_finalizacion(prog->PID, 0, 18, 1);
					}else
					{
						int largo=0;
						char *mensaje_esc = get_mensaje_escritura_info(mensaje,&largo);
						escribir_archivo(largo,offset, mensaje_esc, arch->flag, path, socket_cpu, prog);
						free(mensaje_esc);
					}
					//free(info);
					break;
				case 4: ;//pedido de lectura
					int size = atoi(info);
					pedido_lectura(prog, fd, offset, size, path, socket_cpu);
					break;
				case 6:;
					char *str_fd = get_mensaje(mensaje);
					int fd2= atoi(str_fd);
					cerrar_file(prog->PID, prog->TAP, fd2, socket_cpu);
					free(str_fd);
					break;
				default:
					eliminar_conexion(socket_cpu);
			}
			free(cod);
		}
		else
		{
			forzar_finalizacion(prog->PID, 0, 17, 1);
		}
		free(header);
		free(mensaje);
		free(info);
	}
	//free(path);
}

void escribir_archivo(int largom,int offset, char *info, char *flags, char *path, int socket_cpu, t_program *pr)
{
	if (string_contains(flags, "w"))
	{
		int controlador;
		char *o = string_itoa(offset);
		int size_mensaje=0;
		char *mensaje = armar_info_mensaje(largom,info, path, o,&size_mensaje);
		char *mensaje_enviar = armar_mensaje_pcb("K15", mensaje,size_mensaje);
		enviar(config->cliente_fs, mensaje_enviar, &controlador);
		free(o);
		char *mensaje_recibido = recibir(config->cliente_fs, &controlador);
		char *header = get_header(mensaje_recibido);

		if(comparar_header(header,"F"))
		{
			char *cod = get_codigo(mensaje_recibido);
			int codigo = atoi(cod);
			if(codigo== 5)
			{
				//char *mensaje_leido = get_mensaje(mensaje_recibido);
				enviar(socket_cpu, "OK000000000000000", &controlador);
				//enviar ok 5 ceros
			}else
			{
				forzar_finalizacion(pr->PID, 0, 7, 1);
			}
			free(cod);
		}
		free(header);
		free(mensaje_recibido);
		free(mensaje);
		free(mensaje_enviar);
	}else{
		forzar_finalizacion(pr->PID, 0, 3, 1); //intento escribir en un archivo sin permisos;
	}
}

bool existe_archivo(t_list *tap, int fd)
{
	bool _archivo_TAP(t_TAP *ap)
	{
		return (ap->FD == fd);
	}
	return list_any_satisfy(tap,(void *) _archivo_TAP);
}

void destruir_file(t_TAP *ap)
{
	free(ap->flag);
	free(ap);
}

void destruir_file_TAG(t_TAG *tg)
{
	free(tg->path);
	free(tg);
}

void cerrar_file(int pid, t_list *tap, int fd, int socket_)
{
	bool ex_f = existe_archivo(tap, fd);
	int controlador = 0;

	if(ex_f)
	{
		t_TAP *p_ap = malloc(sizeof(t_TAP));
		p_ap = buscar_archivo_TAP(tap, fd);

		t_TAG *ag = buscar_archivo_TAG_fd(p_ap->FD);
		ag->open_ --;

		bool _archivo_fd(t_TAG *ag2)
		{
			return ag2->FD == fd;
		}

		if(ag->open_ == 0)
		{
			list_remove_and_destroy_by_condition(global_fd, (void *)_archivo_fd, (void *)destruir_file_TAG);
		}

		bool _archivo_TAP(t_TAP *ap)
		{
			return (ap->FD == fd);
		}
		list_remove_and_destroy_by_condition(tap,(void *) _archivo_TAP, (void *) destruir_file);
		enviar(socket_, "OK000000000000000", &controlador);

	}else forzar_finalizacion(pid, 0, 16, 1);
}

char *get_path_msg(char *mensaje, int *payload1)
{
	char *payload = string_substring(mensaje, 13, 4);
	*payload1 = atoi(payload);
	free(payload);
	return string_substring(mensaje, 17, *payload1);
}

char *get_info(char *mensaje, int payload1, int tam_info)
{
	char *payload = string_substring(mensaje, (17+payload1), tam_info);
	int payload2 = atoi(payload);
	free(payload);
	return string_substring(mensaje, (17+payload1+tam_info), payload2);
}

int chequear_respuesta(int socket_cpu, char *path, char *flag, t_program *prog)
{
	int controlador;
	char *mensaje_recibido = recibir(config->cliente_fs, &controlador);
	char *header = get_header(mensaje_recibido);
	int no_existe = 0;

	if(comparar_header(header, "F"))
	{
		char *info = get_mensaje(mensaje_recibido);
		if(!strcmp(info,"ok"))
		{
			/*int fd = abrir_archivo(path, flag, prog);
			char *fd_char = string_itoa(fd);
			char *mensaje = armar_mensaje("K16", fd_char);
			enviar(socket_cpu, mensaje, &controlador);
			free(mensaje);
			free(fd_char);*/
		}
		else if(!(strcmp(info,"no")))
		{
			no_existe = 1;
		}/*else
		{
			char *fd_char = string_itoa(-2);
			char *mensaje = armar_mensaje("K16", fd_char);
			enviar(socket_cpu, mensaje, &controlador);
			free(mensaje);
			free(fd_char);
			no_existe = 2;
		}*/
		free(info);
	}
	else
	{
		char *fd_char = string_itoa(-2);
		char *mensaje = armar_mensaje("K16", fd_char);
		enviar(socket_cpu, mensaje, &controlador);
		free(mensaje);
		free(fd_char);
		no_existe = 2;
	}
	free(mensaje_recibido);
	free(header);
	return no_existe;
}

char *armar_info_mensaje(int largo,char *buffer, char* path, char *off,int *l)
{
	char *mensaje = strdup("");

	char *size_buffer = string_itoa(largo);
	int size_payload = string_length(size_buffer);
	char *sbuffer = string_repeat('0', 4 - size_payload);

	char *size_path = string_itoa(string_length(path));
	int size_payload2 = string_length(size_path);
	char *spath = string_repeat('0', 4 - size_payload2);

	char *size_offset = string_itoa(string_length(off));
	int payload = string_length(size_offset);
	char *soffset = string_repeat('0', 4 - payload);

	char *size_ = string_itoa(string_length(size_buffer));
	int size = string_length(size_);
	char *osize = string_repeat('0', 4 - size);


	string_append(&mensaje, spath);
	string_append(&mensaje, size_path);
	string_append(&mensaje, path);

	string_append(&mensaje, soffset);
	string_append(&mensaje, size_offset);
	string_append(&mensaje, off);

	string_append(&mensaje,osize);
	string_append(&mensaje,size_);
	string_append(&mensaje,size_buffer);

	string_append(&mensaje, sbuffer);
	string_append(&mensaje, size_buffer);
	//string_append(&mensaje, buffer);
	*l = strlen(mensaje)+largo;
	char *final = malloc(strlen(mensaje)+largo);
	memcpy(final, mensaje, strlen(mensaje));
	memcpy(final+strlen(mensaje),buffer,largo);

	free(size_buffer);
	free(sbuffer);
	free(size_path);
	free(spath);
	free(size_offset);
	free(soffset);
	free(size_);
	free(osize);
	free(mensaje);

	return final;
}

void borrar_archivo(int pid, t_list *tap, int fd, int socket_)
{
	t_TAP *tap_ = buscar_archivo_TAP(tap,  fd);
	int controlador = 0;
	if (tap_ != NULL)
	{
		t_TAG *tag = buscar_archivo_TAG_fd(fd);

		if(tag != NULL)
		{
			if (tag->open_ == 1)
			{
				char *eliminar = armar_mensaje("K13", tag->path);
				enviar(config->cliente_fs, eliminar, &controlador);
				char *rec = recibir(config->cliente_fs, &controlador);
				free(eliminar);
				free(rec);
			}

			enviar(socket_, "OK000000000000000", &controlador);
		}
	}
	else
	{
		forzar_finalizacion(pid, 0, 15, 1);
	}
}
