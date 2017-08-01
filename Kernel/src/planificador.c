#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <pthread.h>
#include "planificador.h"
#include "semaforos_vglobales.h"
#include "mensaje_consola.h"
#include "estructuras.h"
#include "cpuManager.h"
#include "metadata.h"
#include "mensaje.h"
#include "memoria.h"
#include "socket.h"
#include "log.h"

extern t_list *list_cpus;
extern t_list *list_ejecutando;
extern t_list *list_finalizados;
extern t_list *list_bloqueados;
extern t_queue *cola_nuevos;
extern t_queue *cola_listos;
extern pthread_mutex_t mutex_lista_cpus;
extern pthread_mutex_t mutex_lista_ejecutando;
extern pthread_mutex_t mutex_lista_finalizados;
extern pthread_mutex_t mutex_lista_bloqueados;
extern pthread_mutex_t mutex_cola_nuevos;
extern pthread_mutex_t mutex_cola_listos;
extern pthread_mutex_t mutex_actualizar_multip;
extern pthread_mutex_t mutex_planificar;
//extern pthread_mutex_t mutex_sincro;
extern sem_t sem_grad_multi;
extern sem_t sem_nuevos;
extern sem_t sem_listos;
extern sem_t sem_cpus;
extern t_configuracion *config;
extern int tam_pagina;
extern int pag_stack;
extern int diferencia_multi;
int controlador;


void finalizar_proceso(int pid, int codigo_finalizacion);
void desbloquear_proceso(int pid);
void bloquear_proceso(int pid, int socket_);
void programas_nuevos_A_listos();
void programas_listos_A_ejecutar();
void forzar_finalizacion(int pid, int cid, int codigo_finalizacion, int aviso);
void agregar_nueva_prog(int id_consola, int pid, char *mensaje, int socket_con);
void finalizar_quantum(int pid);
char *armar_mensaje_memoria(char *mensaje_recibido, int pid);
int calcular_pag(char *mensaje);
int calcular_pag_stack();
void continuar();

void programas_listos_A_ejecutar()
{
	int tam_prog;

	bool _cpuLibre(t_cpu *una_cpu)
	{
		return !(una_cpu->ejecutando);
	}

	while(1)
	{
		sem_wait(&sem_listos);
		sem_wait(&sem_cpus);

		//pthread_mutex_lock(&mutex_sincro);
		continuar();
		tam_prog = 0;

		pthread_mutex_lock(&mutex_lista_cpus);
		t_cpu *cpu_disponible = list_find(list_cpus, (void*)_cpuLibre);
		pthread_mutex_unlock(&mutex_lista_cpus);

		pthread_mutex_lock(&mutex_cola_listos);
		t_program *program = queue_pop(cola_listos);
		pthread_mutex_unlock(&mutex_cola_listos);

		if(program != NULL)
		{
			char *pcb_serializado = serializarPCB_KerCPU(program->pcb,config->algoritmo,config->quantum,config->quantum_sleep,&tam_prog);
			char *mensaje_env = armar_mensaje_pcb("K07", pcb_serializado, tam_prog);

			escribir_log(mensaje_env);

			enviar_pcb(cpu_disponible->socket_cpu, mensaje_env, &controlador, tam_prog+13);
			free(pcb_serializado);

			//Fallo el envio de la pcb a la cpu, se debe eliminar la cpu
			if(controlador>0)
			{
				int i;
				escribir_log_error_con_numero("Ha fallado el envio de una PCB con la CPU: ",cpu_disponible->cpu_id);

				pthread_mutex_lock(&mutex_lista_cpus);
				t_cpu *cpu_disponible_a_borrar = list_remove_by_condition(list_cpus, (void*)_cpuLibre);
				pthread_mutex_unlock(&mutex_lista_cpus);

				free(cpu_disponible_a_borrar);

				pthread_mutex_lock(&mutex_cola_listos);
				int size = queue_size(cola_listos);
				queue_push(cola_listos,program);
				pthread_mutex_unlock(&mutex_cola_listos);

				for(i=0;i<size;i++)
				{
					pthread_mutex_lock(&mutex_cola_listos);
					queue_push(cola_listos,queue_pop(cola_listos));
					pthread_mutex_unlock(&mutex_cola_listos);
				}
				sem_post(&sem_listos);
			}
			else
			{
				pthread_mutex_lock(&mutex_lista_ejecutando);
				list_add(list_ejecutando, program);
				pthread_mutex_unlock(&mutex_lista_ejecutando);

				cpu_disponible->ejecutando = 1; //true
				cpu_disponible->program = program;
			}
			free(mensaje_env);
		}
		else
		{
			//sem_post(&sem_listos);
			sem_post(&sem_cpus);
		}

		//pthread_mutex_unlock(&mutex_sincro);
	}
}

void programas_nuevos_A_listos()
{
	while(1)
	{
		sem_wait(&sem_nuevos);
		sem_wait(&sem_grad_multi);

		//pthread_mutex_lock(&mutex_sincro);

		continuar();

		pthread_mutex_lock(&mutex_cola_nuevos);
		t_nuevo *nuevito = queue_pop(cola_nuevos);
		pthread_mutex_unlock(&mutex_cola_nuevos);

		char *mensaje_envio =  armar_mensaje_memoria(nuevito->codigo, nuevito->pid);

		//envio del codigo a memoria para ver si hay espacio
		enviar(config->cliente_memoria, mensaje_envio, &controlador);

		char *mensaje_recibido = recibir(config->cliente_memoria, &controlador);
		char *cod = get_codigo(mensaje_recibido);
		int codigo_m = atoi(cod);

		if(codigo_m == 3)
		{
			escribir_log_error_con_numero("No se puede guardar codigo en memoria para PID: ",nuevito->pid);
			enviar(nuevito->new_socket, "K05", &controlador);
			forzar_finalizacion(nuevito->pid,0,1,0);
		}
		else
		{
			escribir_log("Se puede guardar codigo en memoria");

			char *char_pid = string_itoa(nuevito->pid);
			char *mensaje_consola = armar_mensaje("K04", char_pid);
			enviar(nuevito->new_socket, mensaje_consola, &controlador);

			int paginas = calcular_pag(nuevito->codigo);
			int len = string_length(nuevito->codigo);
			int resto;
			int n_pag = 0;

			if(len > tam_pagina )
				resto = tam_pagina;
			else
				resto = len;

			while((paginas > n_pag) && (len > 0))
			{
				char *pedazo = string_substring(nuevito->codigo,n_pag*tam_pagina,resto);

				almacenar_bytes(nuevito->pid,n_pag,0,resto, pedazo);
				len = len - tam_pagina;

				if(len > tam_pagina )
					resto = tam_pagina;
				else
					resto = len;

				n_pag ++;
				free(pedazo);
			}

			agregar_nueva_prog(nuevito->consola, nuevito->pid, nuevito->codigo, nuevito->new_socket);

			free(char_pid);
			free(mensaje_consola);
		}

		free(mensaje_envio);
		free(mensaje_recibido);
		free(cod);

		//pthread_mutex_unlock(&mutex_sincro);
	}
}

void agregar_nueva_prog(int id_consola, int pid, char *codigo, int socket_con)
{
	t_program *programa = malloc(sizeof(t_program));
	programa->PID = pid;
	programa->socket_consola = socket_con;
	programa->CID = id_consola;
	programa->allocs = 0;
	programa->frees = 0;
	programa->allocs_size = 0;
	programa->frees_size = 0;
	programa->syscall = 0;
	programa->memoria_dinamica = list_create();
	programa->TAP = list_create();
	programa->semaforos = list_create();
	programa->posiciones = dictionary_create();
	programa->pcb = malloc(sizeof(t_PCB));
	programa->pcb->PC = 0;
	programa->pcb->PID = pid;
	programa->pcb->SP = 0;//ver que es esto!!!
	programa->pcb->exit_code = 1;
	programa->pcb->cant_pag = calcular_pag(codigo);
	programa->pcb->in_cod = armarIndiceCodigo(codigo);
	programa->pcb->in_et = armarIndiceEtiquetas(codigo);
	programa->pcb->in_stack = armarIndiceStack(codigo);

	escribir_log("Se ha movido un proceso de nuevos a listos");

	continuar();

	pthread_mutex_lock(&mutex_cola_listos);
	queue_push(cola_listos, programa);
	pthread_mutex_unlock(&mutex_cola_listos);

	sem_post(&sem_listos);

	free(codigo);
}

void bloquear_proceso(int pid, int socket_)
{
	//pthread_mutex_lock(&mutex_sincro);

	continuar();

	bool _buscar_proceso(t_program *un_proceso)
	{
		return (pid == un_proceso->PID);
	}

	pthread_mutex_lock(&mutex_lista_ejecutando);
	t_program *proc = list_remove_by_condition(list_ejecutando, (void*)_buscar_proceso);
	pthread_mutex_unlock(&mutex_lista_ejecutando);

	escribir_log_con_numero("Se ha bloqueado el proceso: ", proc->PID);

	pthread_mutex_lock(&mutex_lista_bloqueados);
	list_add(list_bloqueados, proc);
	pthread_mutex_unlock(&mutex_lista_bloqueados);

	t_cpu *cpu_activa = buscar_cpu(socket_);
	cpu_activa->ejecutando = 0;
	sem_post(&sem_cpus);

	//pthread_mutex_unlock(&mutex_sincro);
}

void desbloquear_proceso(int pid)
{
	//pthread_mutex_lock(&mutex_sincro);

	continuar();

	bool _buscar_proceso(t_program *un_proceso)
	{
		return (pid == un_proceso->PID);
	}

	pthread_mutex_lock(&mutex_lista_bloqueados);
	t_program *proc = list_remove_by_condition(list_bloqueados, (void*)_buscar_proceso);
	pthread_mutex_unlock(&mutex_lista_bloqueados);

	escribir_log_con_numero("Se ha desbloqueado el proceso: ", proc->PID);

	pthread_mutex_lock(&mutex_cola_listos);
	queue_push(cola_listos, proc);
	pthread_mutex_unlock(&mutex_cola_listos);

	sem_post(&sem_listos);

	//pthread_mutex_unlock(&mutex_sincro);
}

void finalizar_proceso(int pid, int codigo_finalizacion)
{
	//pthread_mutex_lock(&mutex_sincro);

	continuar();

	bool _buscar_proceso(t_program *un_proceso)
	{
		return (pid == un_proceso->PID);
	}

	bool _encontrame_cpu(t_cpu *cpu)
	{
		return cpu->program->PID == pid;
	}

	t_cpu *cpu = list_find(list_cpus, (void*)_encontrame_cpu);

	pthread_mutex_lock(&mutex_lista_ejecutando);
	t_program *programa = list_remove_by_condition(list_ejecutando, (void*)_buscar_proceso);
	pthread_mutex_unlock(&mutex_lista_ejecutando);

	programa->pcb->exit_code = codigo_finalizacion;

	escribir_log_con_numero("Se ha finalizado el proceso: ", programa->PID);
	avisar_consola_proceso_murio(programa);

	/*if(!(list_is_empty(programa->memoria_dinamica)))
		list_destroy_and_destroy_elements(programa->memoria_dinamica, (void *)liberar_pagina);*/
	sem_signal(programa, "", cpu->socket_cpu, 1);
	liberar_proceso_pagina(programa->PID);

	pthread_mutex_lock(&mutex_lista_finalizados);
	list_add(list_finalizados, programa);
	pthread_mutex_unlock(&mutex_lista_finalizados);

	sem_post(&sem_cpus);
	sem_post(&sem_grad_multi);

	//pthread_mutex_unlock(&mutex_sincro);
}

void forzar_finalizacion(int pid, int cid, int codigo_finalizacion, int aviso)
{
	t_list *procesos = list_create();
	t_list *procesos_ejecutando = list_create();
	int i, contador = 0;

	continuar();

	bool _buscar_program(t_program *pr)
	{
		if(pid)
			return pr->PID == pid;
		else
			return pr->CID == cid;
	}

	bool _buscar_nuevito(t_nuevo *pr)
	{
		if(pid)
			return pr->pid == pid;
		else
			return pr->consola == cid;
	}

	void _finalizar_proceso_ejecutando(t_program *pr)
	{
		pedir_pcb_error(pr,codigo_finalizacion);
	}

	void _finalizar_proceso(t_program *pr)
	{
		/*
		bool _encontrame_cpu(t_cpu *cpu)
		{
			return cpu->program->PID == pr->PID;
		}

		t_cpu *cpu = list_find(list_cpus, (void*)_encontrame_cpu);
		*/
		pr->pcb->exit_code = (-1)*codigo_finalizacion;
		if(aviso) avisar_consola_proceso_murio(pr);

		list_destroy_and_destroy_elements(pr->memoria_dinamica, (void *)liberar_pagina);

		sem_signal(pr, "", 0, 1);
		liberar_proceso_pagina(pr->PID);

		pthread_mutex_lock(&mutex_lista_finalizados);
		list_add(list_finalizados,pr);
		pthread_mutex_unlock(&mutex_lista_finalizados);

		sem_post(&sem_grad_multi);
	}

	contador = list_count_satisfying(list_ejecutando, (void*)_buscar_program);
	while(contador)
	{
		pthread_mutex_lock(&mutex_lista_ejecutando);
		list_add(procesos_ejecutando,list_remove_by_condition(list_ejecutando, (void*)_buscar_program));
		pthread_mutex_unlock(&mutex_lista_ejecutando);
		contador --;
	}

	contador = list_count_satisfying(list_bloqueados, (void*)_buscar_program);
	while(contador)
	{
		pthread_mutex_lock(&mutex_lista_bloqueados);
		list_add(procesos, list_remove_by_condition(list_bloqueados, (void*)_buscar_program));
		pthread_mutex_unlock(&mutex_lista_bloqueados);
		contador --;
	}

	controlador = queue_size(cola_nuevos);
	for(i=0;i<controlador;i++)
	{
		pthread_mutex_lock(&mutex_cola_nuevos);
		t_nuevo *prog = queue_pop(cola_nuevos);
		pthread_mutex_unlock(&mutex_cola_nuevos);

		if(_buscar_nuevito(prog))
		{
			list_add(procesos,prog);
			//sem_wait(&sem_nuevos);
		}
		else
		{
			pthread_mutex_lock(&mutex_cola_nuevos);
			queue_push(cola_nuevos,prog);
			pthread_mutex_unlock(&mutex_cola_nuevos);
		}
	}

	controlador = queue_size(cola_listos);
	for(i=0;i<controlador;i++)
	{
		pthread_mutex_lock(&mutex_cola_listos);
		t_program *prog = queue_pop(cola_listos);
		pthread_mutex_unlock(&mutex_cola_listos);

		if(_buscar_program(prog))
		{
			list_add(procesos,prog);
			//sem_wait(&sem_listos);
		}
		else
		{
			pthread_mutex_lock(&mutex_cola_listos);
			queue_push(cola_listos,prog);
			pthread_mutex_unlock(&mutex_cola_listos);
		}
	}

	list_iterate(procesos_ejecutando, (void*)_finalizar_proceso_ejecutando);
	list_iterate(procesos, (void*)_finalizar_proceso);
	list_add_all(list_ejecutando,procesos_ejecutando);

	list_destroy(procesos);
	list_destroy(procesos_ejecutando);

	//pthread_mutex_unlock(&mutex_sincro);
}

int calcular_pag(char *mensaje)
{
	int tamanio = string_length(mensaje);
	int paginas = (int)(tamanio/tam_pagina);

	if (tamanio % tam_pagina > 0)
	{
		paginas ++;
	}

	return paginas;
}

void finalizar_quantum(int pid)
{
	//pthread_mutex_lock(&mutex_sincro);

	bool _buscar_proceso(t_program *un_proceso)
	{
		return (pid == un_proceso->PID);
	}

	continuar();

	pthread_mutex_lock(&mutex_lista_ejecutando);
	t_program *programa = list_remove_by_condition(list_ejecutando, (void*)_buscar_proceso);
	pthread_mutex_unlock(&mutex_lista_ejecutando);

	pthread_mutex_lock(&mutex_cola_listos);
	queue_push(cola_listos, programa);
	pthread_mutex_unlock(&mutex_cola_listos);

	sem_post(&sem_listos);
	sem_post(&sem_cpus);

	//pthread_mutex_unlock(&mutex_sincro);
}

void actualizar_grado_multiprogramacion()
{
	while(1)
	{
		pthread_mutex_lock(&mutex_actualizar_multip);

		while(diferencia_multi != 0)
		{
			if(diferencia_multi>0)
			{
				sem_post(&sem_grad_multi);
				diferencia_multi--;
			}
			else
			{
				sem_wait(&sem_grad_multi);
				diferencia_multi++;
			}
		}
	}
}

char *armar_mensaje_memoria(char *mensaje_recibido, int pid)
{
	char *resultado = strdup("K06");

	char *pid_aux = string_itoa(pid);
	int size_pid = string_length(pid_aux);
	char *completar = string_repeat('0', 4 - size_pid);

	int paginas = calcular_pag(mensaje_recibido);
	char *pag_char = string_itoa(paginas);
	int size_paginas = string_length(pag_char);
	char *completar2 = string_repeat('0', 4 - size_paginas);

	int paginas_stack = calcular_pag_stack();
	char *pag_st = string_itoa(paginas_stack);
	int size_pag_st = string_length(pag_st);
	char *completar3 = string_repeat('0', 4 - size_pag_st);

	string_append(&resultado, completar);
	string_append(&resultado, pid_aux);
	string_append(&resultado, completar2);
	string_append(&resultado, pag_char);
	string_append(&resultado, completar3);
	string_append(&resultado, pag_st);

	free(pid_aux);
	free(completar);
	free(completar2);
	free(pag_char);
	free(pag_st);
	free(completar3);

	return resultado;
}

int calcular_pag_stack()
{
	pag_stack = config->stack_size;
	return pag_stack;
}

//Esta funcion se utiliza para chequear si la planificacion debe continuar
void continuar()
{
	pthread_mutex_lock(&mutex_planificar);
	pthread_mutex_unlock(&mutex_planificar);
}
