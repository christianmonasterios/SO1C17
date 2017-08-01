#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "memoria.h"
#include "estructuras.h"
#include "configuracion.h"
#include "socket.h"
#include "manejo_errores.h"
#include "log.h"
#include "mensaje.h"
#include "consolaManager.h"
#include "planificador.h"

extern t_configuracion *config;
extern int tam_pagina;
extern int pag_stack;
int posicion_pagina;
//int inicio_bloque;

void handshakearMemory();
void reservar_memoria_din(t_program *program, int size_solicitado, int so_cpu);
void inicializar_pagina_dinamica(t_program *prog, int size_sol);
int ubicar_bloque(t_pagina *pagina, int tam_sol, t_program *program, int so_cpu);
void _free_bloque(t_bloque *bloque);
int pedir_pagina();
t_pagina *_buscar_pagina(t_list *mem, int num_pag);
void destruir_heap(t_bloque *bl);
void liberar_pagina(t_pagina *pagina);
int chequear_pagina(t_pagina *page);
int almacenar_bytes(int pid, int numpag, int offset, int tam, char *buffer);
char *buffer_bloque(int size, int booleano);
void *pedir_bloque_libre(t_pagina *pagina, int pid, int tam_sol, int *inicio_bloque);
char *pedir_bytes_memoria(int pid, int numpag, int offset);
HeapMetadata *armar_metadata(char *metadata);
void liberar_proceso_pagina(int pid);
void compactar_contiguos(int pid, t_pagina *pagina);
void liberar_bloque(t_program *prog, char *offset, int socket_);

void handshakearMemory()
{
	int controlador = 0;
	char res[13];

	enviar(config->cliente_memoria, "K00", &controlador);
	recv(config->cliente_memoria, res, 13, 0);
	//respuesta = recibir(config->cliente_memoria, &controlador);
	char *prim = strdup(res);
	char *str_size = string_substring(prim, 3, 10);
	int size = atoi(str_size);

	char res2[size];

	recv(config->cliente_memoria, res2, size, 0);

	char *respuesta = strdup(prim);
	string_append(&respuesta,res2);

	char *codigo = get_codigo(respuesta);
	int cod = atoi(codigo);
	char *mensaje33 = get_mensaje(respuesta);

	if(cod == 0)
		tam_pagina = atoi(mensaje33);

	free(codigo);
	free(mensaje33);
	free(respuesta);
	free(prim);
	free(str_size);
}

void inicializar_pagina_dinamica(t_program *prog, int size_sol)
{
	t_pagina *pagina = malloc(sizeof(t_pagina));
	pagina->heaps = list_create();
	pagina->n_pagina = prog->pcb->cant_pag + pag_stack + list_size(prog->memoria_dinamica);
	pagina->esp_libre = tam_pagina - 5;

	HeapMetadata *heap = malloc(sizeof(HeapMetadata));
	heap->size = tam_pagina - 5;
	heap->isFree = 1;

	t_bloque *bloque = malloc(sizeof(t_bloque));
	bloque->metadata = heap;

	list_add(pagina->heaps, bloque);
	list_add(prog->memoria_dinamica, pagina);

	char *buffer = buffer_bloque(heap->size, heap->isFree);

	int resultado = almacenar_bytes(prog->PID,pagina->n_pagina,0,5,buffer);
	free(buffer);

	if(resultado==3)
	{
		forzar_finalizacion(prog->PID,0,5,0);
	}
}

void reservar_memoria_din(t_program *program, int size_solicitado, int so_cpu)
{
	int ubicado = 0;
	program->syscall++;

	if (size_solicitado < (tam_pagina -10))
	{
		if (!list_is_empty(program->memoria_dinamica))
		{
			int size_disponible;
			int n = 0;
			int size_lpages = list_size(program->memoria_dinamica);

			while(n < size_lpages && ubicado == 0)
			{
				t_pagina *page = list_get(program->memoria_dinamica, n);
				size_disponible = page->esp_libre;

				if((size_disponible - 5) >= size_solicitado)
				{
					ubicado = ubicar_bloque(page, size_solicitado, program, so_cpu);

					if(ubicado == 0)
					{
						compactar_contiguos(program->PID, page);
						ubicado = ubicar_bloque(page, size_solicitado, program, so_cpu);
					}
				}
				else n++;
			}
			if(ubicado==0)
			{
				int pedido = pedir_pagina(program);

				if(pedido == 1)
				{
					inicializar_pagina_dinamica(program, size_solicitado);
					program->syscall --;
					reservar_memoria_din(program,size_solicitado,so_cpu);
				}
				else
					forzar_finalizacion(program->PID, 0, 5, 0);
			}
		}
		else
		{
			int pedido = pedir_pagina(program);

			if(pedido == 1)
			{
				inicializar_pagina_dinamica(program, size_solicitado);
				program->syscall --;
				reservar_memoria_din(program,size_solicitado,so_cpu);
			}
			else
				forzar_finalizacion(program->PID, 0, 5, 0);
		}
	}
	else
	{
		forzar_finalizacion(program->PID, 0, 8, 0);
		ubicado = 0;
	}
}

int ubicar_bloque(t_pagina *pagina, int tam_sol, t_program *program, int so_cpu)
{
	int inicio_bloque;
	t_bloque *bloque = pedir_bloque_libre(pagina, program->PID, tam_sol, &inicio_bloque);

	if (bloque != NULL)
	{
		program->allocs++;
		program->allocs_size = program->allocs_size + tam_sol;

		int sz = bloque->metadata->size;
		bloque->metadata->isFree = 0;
		bloque->metadata->size = tam_sol;

		t_infoheap *infheap = malloc(sizeof(t_infoheap));
		infheap->bloque = posicion_pagina;
		infheap->pagina = pagina->n_pagina;
		int offset = pagina->n_pagina * tam_pagina + inicio_bloque + 5;
		char *offs_ = string_itoa(offset);
		dictionary_put(program->posiciones, offs_ ,infheap);
		char *mens = armar_mensaje("K99", offs_);
		int contr = 0;

		char *buffer = buffer_bloque(bloque->metadata->size, 0);
		int respuesta = almacenar_bytes(program->PID, pagina->n_pagina, inicio_bloque, 5, buffer);
		free(buffer);

		if(respuesta==3)
		{
			forzar_finalizacion(program->PID,0,5,0);
			return 0;
		}

		enviar(so_cpu, mens, &contr);
		free(mens);
		free(offs_);

		if (tam_sol < sz)
		{
			t_bloque *bl = malloc(sizeof(t_bloque));
			bl->metadata = malloc(sizeof(HeapMetadata));
			bl->metadata->isFree = 1;
			bl->metadata->size = sz - tam_sol - 5;

			char *buffer = buffer_bloque(bl->metadata->size, 1);
			almacenar_bytes(program->PID, pagina->n_pagina, (inicio_bloque + tam_sol + 5), 5, buffer);
			free(buffer);

			//list_add_in_index(pagina->heaps, posicion_pagina, bl);
			pagina->esp_libre = pagina->esp_libre - (tam_sol+5);
			free(bl);
		}
		free(bloque);
		return 1; //offset;
	}
	else
	{
		//free(bloque);
		return 0;
	}
}

void _free_bloque(t_bloque *bloque)
{
	free(bloque->data);
	free(bloque->metadata);
	free(bloque);
}

int pedir_pagina(t_program *program)
{
	int res = 0;
	int controlador = 0;
	char *mensaje = strdup("K19");

	char *pid_aux = string_itoa(program->PID);
	int size_pid = string_length(pid_aux);
	char *completar = string_repeat('0', 4 - size_pid);

	string_append(&mensaje, completar);
	string_append(&mensaje, pid_aux);
	string_append(&mensaje, "0001");

	enviar(config->cliente_memoria, mensaje, &controlador);

	if (controlador > 0)
		return 0;
	else
	{
		char *respuesta = recibir(config->cliente_memoria, &controlador);
		char *codigo = get_codigo(respuesta);
		int cod = atoi(codigo);

		if(cod == 2) res = 1;

		free(respuesta);
		free(codigo);
	}

	free(mensaje);
	free(completar);
	free(pid_aux);

	return res;
}

void liberar_bloque(t_program *prog, char *offset, int socket_)
{
	prog->syscall++;

	t_infoheap *heap = dictionary_get(prog->posiciones, offset);
    t_pagina *page = _buscar_pagina(prog->memoria_dinamica, heap->pagina);

	if(page != NULL)
	{
		prog->frees++;
		int off_ = atoi(offset);
		char *bytes = pedir_bytes_memoria(prog->PID, page->n_pagina, (off_-5)%tam_pagina);
		char *men = get_mensaje(bytes);
		HeapMetadata *meta = armar_metadata(men);
		t_bloque *bloque = malloc(sizeof(t_bloque));//list_get(page->heaps, heap->bloque);
		bloque->metadata = meta;
		//free(bloque->data);
		prog->frees_size = prog->frees_size + bloque->metadata->size;
		bloque->metadata->isFree = true;
		page->esp_libre = page->esp_libre + bloque->metadata->size + 5;

		int controlador3;

		char *buffer = buffer_bloque(bloque->metadata->size, 1);
		int almacenar = almacenar_bytes(prog->PID, page->n_pagina, (off_-5)%tam_pagina, 5, buffer);

		if (almacenar == 2)
		{
			enviar(socket_, "OK00000000000", &controlador3);
		}
		else forzar_finalizacion(prog->PID, 0, 5, 1);

		free(buffer);
		free(men);
		if(page->esp_libre == tam_pagina - 5)
		{
			//list_remove_and_destroy_element(prog->memoria_dinamica, (page->n_pagina - prog->pcb->cant_pag - pag_stack),(void *) liberar_pagina);

			int controlador;
			char *mensaje = strdup("K24");
			char *pid_aux = string_itoa(prog->PID);
			int size_pid = string_length(pid_aux);
			char *completar_pid = string_repeat('0', 4 - size_pid);
			char *pagina = string_itoa(page->n_pagina);
			int size_pagina = string_length(pagina);
			char *completar = string_repeat('0', 4 - size_pagina);
			string_append(&mensaje, completar_pid);
			string_append(&mensaje, pid_aux);
			string_append(&mensaje, completar);
			string_append(&mensaje, pagina);

			enviar(config->cliente_memoria, mensaje, &controlador);

			char *reccv = recibir(config->cliente_memoria, &controlador);
			free(reccv);

			free(completar_pid);
			free(pid_aux);
			free(completar);
			free(pagina);
			free(mensaje);
		}
	}
	else
	{
		forzar_finalizacion(prog->PID, 0, 5, 0);//el error puede estar mal!
	}
}

void compactar_contiguos(int pid, t_pagina *pagina)
{
	int size_anterior, bolean_anterior;
	int fin_raiz = 1;
	int offset = 0;
	int offset_in = 0;

	while(fin_raiz)
	{
		size_anterior = 0;
		bolean_anterior = 0;
		fin_raiz = 0;
		offset = 0;

		while(offset < tam_pagina)
		{
			char *info_bloque = pedir_bytes_memoria(pid, pagina->n_pagina, offset);
			char *metadata = get_mensaje(info_bloque);
			HeapMetadata *heapMeta = armar_metadata(metadata);

			if((heapMeta->isFree)&&(bolean_anterior))
			{
				int size = heapMeta->size + size_anterior + 5;
				char *buffer = buffer_bloque(size, 1);
				almacenar_bytes(pid, pagina->n_pagina, offset_in, 5, buffer);

				fin_raiz = 1;
				free(buffer);
				break;
			}
			else if(heapMeta->isFree)
			{
				size_anterior = heapMeta->size;
				bolean_anterior = heapMeta->isFree;
				offset_in = offset;
			}
			else//porque es false
			{
				bolean_anterior = 0;
			}

			offset = offset + heapMeta->size + 5;
			free(info_bloque);
			free(metadata);
			free(heapMeta);
		}
	}
}

void liberar_pagina(t_pagina *pagina)
{
	if(list_size(pagina->heaps) != 0)
		list_destroy_and_destroy_elements(pagina->heaps, (void *)destruir_heap);

	free(pagina);
}

void liberar_proceso_pagina(int pid)
{
	int contr = 0;
	char *men_env = strdup("K25");

	char *pid_ = string_itoa(pid);
	int len = string_length(pid_);
	char *completar = string_repeat('0', 4 - len);

	string_append(&men_env, completar);
	string_append(&men_env, pid_);

	enviar(config->cliente_memoria, men_env, &contr);

	char *rec = recibir(config->cliente_memoria, &contr);
	free(rec);

	free(men_env);
	free(pid_);
	free(completar);
}

void destruir_heap(t_bloque *bl)
{
	if(bl != NULL)
	{
		if (bl->metadata != NULL)
		{
			free(bl->metadata);
		}
		free(bl);
	}
}

t_pagina *_buscar_pagina(t_list *mem, int num_pag)
{
	bool _pagina(t_pagina *pag)
	{
		return pag->n_pagina == num_pag;
	}

	return list_find(mem,(void *)_pagina);
}

int chequear_pagina(t_pagina *page)
{
	bool _esta_libre(t_bloque *bloque)
	{
		return bloque->metadata->isFree;
	}

	if(list_all_satisfy(page->heaps, (void *)_esta_libre))
	{
		return 1;
	}
	else
		return 0;
}

void lib_bloque(t_bloque *bl)
{
	free(bl->metadata);
	free(bl);
}

/*void juntar_memoria(t_list *hp, t_bloque *blo, t_bloque *blo_liberado, int num_bloque, bool anterior)
{
	if(anterior)
	{
		blo->metadata->size =+ blo_liberado->metadata->size;
		free(blo->data);
		blo->data = malloc(blo->metadata->size);

		list_remove_and_destroy_element(hp, num_bloque, (void *)lib_bloque);
	}
	else
	{
		blo_liberado->metadata->size =+ blo->metadata->size;
		blo_liberado->data = malloc(blo_liberado->metadata->size);

		list_remove_and_destroy_element(hp, num_bloque - 1, (void *)lib_bloque);
	}
}*/

int almacenar_bytes(int pid, int numpag, int offset, int tam, char *buffer)
{
	int contr = 0;
	char *men_env = strdup("K90");

	char *pid_ = string_itoa(pid);
	int len = string_length(pid_);
	char *completar = string_repeat('0', 4 - len);

	string_append(&men_env, completar);
	string_append(&men_env, pid_);

	free(completar);

	char *numpag_ = string_itoa(numpag);
	len = string_length(numpag_);
	completar = string_repeat('0', 4 - len);

	string_append(&men_env, completar);
	string_append(&men_env, numpag_);

	free(completar);

	char *offset_ = string_itoa(offset);
	len = string_length(offset_);
	completar = string_repeat('0', 4 - len);

	string_append(&men_env, completar);
	string_append(&men_env, offset_);

	free(completar);

	char *tam_ = string_itoa(tam);
	len = string_length(tam_);
	completar = string_repeat('0', 4 - len);

	string_append(&men_env, completar);
	string_append(&men_env, tam_);

	free(completar);
	string_append(&men_env, buffer);

	enviar(config->cliente_memoria, men_env, &contr);

	char *men_rec = recibir(config->cliente_memoria, &contr);

	free(men_env);
	free(pid_);
	free(numpag_);
	free(offset_);
	free(tam_);

	char *codigo = get_codigo(men_rec);
	int int_codigo = atoi(codigo);

	if(int_codigo==3)
		escribir_log_error_con_numero("No se pudieron alocar los bytes para el PID: ",pid);
	else
		escribir_log_con_numero("Se logro alocar los bytes para el PID: ",pid);

	free(codigo);
	free(men_rec);
	return int_codigo;
}

char *pedir_bytes_memoria(int pid, int numpag, int offset)
{
	int contr = 0;
	char *men_env = strdup("K94");

	char *pid_ = string_itoa(pid);
	int len = string_length(pid_);
	char *completar = string_repeat('0', 4 - len);

	string_append(&men_env, completar);
	string_append(&men_env, pid_);

	free(completar);

	char *numpag_ = string_itoa(numpag);
	len = string_length(numpag_);
	completar = string_repeat('0', 4 - len);

	string_append(&men_env, completar);
	string_append(&men_env, numpag_);

	free(completar);

	char *offset_ = string_itoa(offset);
	len = string_length(offset_);
	completar = string_repeat('0', 4 - len);

	string_append(&men_env, completar);
	string_append(&men_env, offset_);

	free(completar);

	char *tam = strdup("0005");
	string_append(&men_env, tam);

	enviar(config->cliente_memoria, men_env, &contr);

	free(men_env);
	free(pid_);
	free(numpag_);
	free(offset_);
	free(tam);
	return recibir(config->cliente_memoria, &contr);
}

char *buffer_bloque(int size, int booleano)
{
	char *metadata = string_itoa(size);
	int meta_len = string_length(metadata);
	char* completar = string_repeat('0', 4 - meta_len);

	char *char_meta = string_itoa(booleano);

	char *buffer = strdup(completar);
	string_append(&buffer, metadata);
	string_append(&buffer, char_meta);

	free(metadata);
	free(completar);
	free(char_meta);

	return buffer;
}

void *pedir_bloque_libre(t_pagina *pagina, int pid, int tam_sol, int *inicio_bloque)
{
	*inicio_bloque = 0;
	int encontrado = 0;
	int fin = 0;
	int offset = 0;
	t_bloque *bloque;
	//char *pedir_bytes_memoria(int pid, int numpag, int offset)

	while(encontrado == 0 && fin == 0)
	{
		char *info_bloque = pedir_bytes_memoria(pid, pagina->n_pagina, offset);
		char *metadata = get_mensaje(info_bloque);
		HeapMetadata *heapMeta = armar_metadata(metadata);

		if(heapMeta->isFree && heapMeta->size >= tam_sol)
		{
			encontrado = 1;
			bloque = malloc(sizeof(t_bloque));
			bloque->metadata = heapMeta;
		}
		else
		{
			offset = offset + (5 + heapMeta->size);
			*inicio_bloque = offset;
			if(offset > tam_pagina)
				fin = 1;
			free(heapMeta);
		}

		free(info_bloque);
		free(metadata);
	}

	if (fin)
		bloque = NULL;

	return bloque;
}

HeapMetadata *armar_metadata(char *metadata)
{
	HeapMetadata *metadaHeap = malloc(sizeof(HeapMetadata));
	char *cantidad = string_substring(metadata, 0, 4);
	int cant = atoi(cantidad);
	free(cantidad);
	char *free_ = string_substring(metadata, 4, 1);
	bool is_free;

	if (atoi(free_))
		is_free = true;
	else
		is_free = false;

	metadaHeap->isFree = is_free;
	metadaHeap->size = cant;
	
	free(free_);
	return metadaHeap;
}
