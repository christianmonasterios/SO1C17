#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>

typedef struct
{
	char *ip_kernel;
	int puerto_prog;
	int puerto_cpu;
	char *ip_memoria;
	int puerto_memoria;
	char *ip_fs;
	int puerto_fs;
	int quantum;
	int quantum_sleep;
	char *algoritmo;
	int grado_multiprog;
	char **sem_ids; //alfanumérico
	char **sem_init; //numérico
	char **shared_vars; //alfanumérico
	int stack_size;
	int server_cpu;
	int server_consola;
	int cliente_fs;
	int cliente_memoria;
}t_configuracion;

typedef struct
{
	int offset_inicio;
	int offset_fin;
}t_sentencia;

typedef struct
{
	int PID;
	int PC;
	int cant_pag;
	int SP;
	t_sentencia* in_cod;
	char* in_et;
	t_list* in_stack;  //lista de t_stack_element
	int exit_code;
}t_PCB;

typedef struct
{
	char ID;
	int pag;
	int offset;
	int size;
}__attribute__((__packed__))t_memoria;

typedef struct
{
	int FD;
	char *flag;
	int GFD;
}t_TAP; //tabla de archivo por proceso

typedef struct
{
	int PID;
	int CID;
	int socket_consola;
	int syscall;
	int allocs;
	int frees;
	int allocs_size;
	int frees_size;
	t_PCB *pcb;
	t_list *semaforos;
	t_list *TAP; //tabla de archivo por proceso
	t_list *memoria_dinamica; //lista de paginas pedidas de manera dinámica
	t_dictionary *posiciones;
}t_program; //cada vez que se crea un programa, además del pcb esta estructura de control

typedef struct
{
	int pos;
	t_list *args; //lista de t_memoria
	t_list *vars; //idem args
	int retPos; //posición en el indice de código, empezando de 0, donde retorna la función
	t_memoria retVar;
}t_stack_element;

typedef struct
{
	int n_pagina;
	t_list *heaps; //de bloques
	int esp_libre;
}t_pagina;

/*typedef struct
{
	t_pagina *pagina;
	t_PCB *pcb;
	int esp_libre;
}t_heap; //con esto haríamos una lista de las paginas que se solicitó de manera dinámica
*/
typedef struct
{
	int size;
	bool isFree;//1:true  0:false
}HeapMetadata;

typedef struct
{
	HeapMetadata *metadata;
	void *data;
}t_bloque;

typedef struct
{
	int cpu_id;
	int socket_cpu;
	bool ejecutando;
	t_program *program;
}t_cpu;

typedef struct
{
	int CID;
	int socket;
}t_consola;

typedef struct
{
	int value;
	t_queue *procesos;
}t_sem;

typedef struct
{
	char *path;
	int FD;
	int open_;
}t_TAG;

typedef struct
{
	int value;
	int mutex_;
	t_queue *procesos;
}t_vglobal;

typedef struct
{
	int pagina;
	int bloque;
}t_infoheap;

typedef struct
{
	int pid;
	int new_socket;
	int consola;
	char *codigo;
}t_nuevo;

#endif /* ESTRUCTURAS_H_ */
