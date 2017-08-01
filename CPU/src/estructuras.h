#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

#define CONTINUAR 0
#define FINALIZAR_PROGRAMA 1
#define FINALIZAR_POR_QUANTUM 2
#define FINALIZAR_POR_ERROR 3
#define BLOQUEAR_PROCESO 5
#define SALTO_LINEA 1


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
	int PID;
	int PC;
	int cant_pag;
	int SP;
	t_sentencia* in_cod;
	t_dictionary* dicc_et;
	char* in_et;
	t_list* in_stack;  //lista de t_stack_element
	int exit_code;
	char* algoritmo;
	int quantum;
	int quantum_sleep;

}t_PCB_CPU;

typedef struct
{
	char ID;
	int pag;
	int offset;
	int size;
}__attribute__((__packed__))t_memoria;
typedef struct
{
	int pos;
	t_list* args; //lista de t_memoria
	t_list* vars; //idem args
	int retPos; //posición en el indice de código, empezando de 0, donde retorna la función
	t_memoria retVar;
}t_stack_element;

#endif /* ESTRUCTURAS_H_ */
