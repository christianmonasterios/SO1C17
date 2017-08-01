

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <parser/metadata_program.h>
#include "log.h"
#include "socket.h"
#include "funcionesCPU.h"
#include "mensajes.h"
#include "funcionesParser.h"
#include "estructuras.h"


char *ipK;
char *ipM;

int puertoK;
int puertoM;
int accion_siguiente;
int sockKerCPU;
int sockMemCPU;
int tam_pagina_memoria;
int fifo;
int salto_linea;
int ABORTAR;
int con_PCB;

t_PCB_CPU *pcb;
//char* programa;


/*
static const char* prueba_ansisop =
		"#!/usr/bin/ansisop\n"
		"begin\n"
		"variables a\n"
		"a = 2\n"
		"wait SEM1\n"
		"prints n !Global\n"
		"!Global = !Global + a\n"
		"signal SEM1\n"
		"prints n !Global\n"
		"end\n";
static const char* bobo_ansisop =
		"#!/usr/bin/ansisop\n"
		"begin\n"
		"variables a\n"
		"a = 2\n"
		"prints n a\n"
		"end\n"
		"\n";
static const char* compartida_ansisop =
		"#!/usr/bin/ansisop\n"
		"begin\n"
		"variables a\n"
		"a = 2\n"
		"!colas = a\n"
		"end\n"
		"\n";
static const char* facil_ansisop =
		"begin\n"
		"variables a, b\n"
		"a = 3\n"
		"b = 5\n"
		"a = b + 12\n"
		"end\n"
		"\n";
static const char* otro_ansisop =
		"#!/usr/bin/ansisop\n"
		"begin\n"
		"# primero declaro las variables\n"
		"variables a, b\n"
		"a = 20\n"
		"print a\n"
		"end\n"
		"\n";
static const char* con_funcion_ansisop =
		"#!/usr/bin/ansisop\n"
		"b-egin variables a,g\n"
		"a = 1\n"
		"g <- doble a\n"
		"prints g\n"
		"variables i,b\n"
		"i = 1\n"
		":inicio_for\n"
		"i = i + 1\n"
		"jnz i inicio_for\n"
		"#fuera del for\n"
		"end\n"
		"function doble\n"
		"variables f\n"
		"f = $0 + $0\n"
		"return f\n"
		"end\n"
		"\n";

*/

void procesar();
//void iniciar_pcb_falsa();
void leerArchivoConfiguracion(char* argv);
int conexion_Kernel(int puertoK, char* ipK);
int conexion_Memoria(int puertoM,char* ipM);
char * pedir_linea_memoria();
void finalizar_por_senial(int);
void liberar_pcb();
//char *pedir_linea_memoria2();

int main(int argc, char *argv[])
{

	//char *programa =strdup(facil_ansisop);
	//char *programa =strdup(otro_ansisop);
	//programa =strdup(con_funcion_ansisop);
	//programa = strdup(prueba_ansisop);
	char *ruta_log = strdup("/home/utnso/CPUlog");
	crear_archivo_log(ruta_log);
	leerArchivoConfiguracion(argv[1]);

	signal(SIGUSR1,finalizar_por_senial);
	signal(SIGINT,finalizar_por_senial);

	con_PCB = 0;
	salto_linea = 0;
	accion_siguiente = CONTINUAR;
	ABORTAR = 0;

	int res;
	int chau = 0;


	res = conexion_Kernel(puertoK, ipK);
	if(res != 0){
		cerrar_conexion(sockKerCPU);
		escribir_log("CPU finalizada por error en conexión con Kernel",2);
		chau=1;
		goto fin;
	}

	res = conexion_Memoria(puertoM,ipM);
	if(res!= 0){
		cerrar_conexion(sockMemCPU);
		escribir_log("CPU finalizada por error en conexión con Memoria",2);
		chau=1;
		goto fin;
	}

	int controlador = 0;
	char *buff = malloc(13);
	char *idmensaje;
	char *sizemensaje;

	while(chau!=1){

		memset(buff,'\0',13);

		int largomensaje  = 0;
		escribir_log("Esperando mensajes Kernel para ponerme a trabajar... ",1);

		recibir(sockKerCPU,&controlador,buff,13);

		if(controlador != 0){
			escribir_log("error recibiendo mensaje del Kernel, bai",2);
			chau = 1;
			goto fin2;
		}

		idmensaje = string_substring(buff,1,2);

		switch (atoi(idmensaje)){
		case 07:
			con_PCB = 1;

			sizemensaje = string_substring(buff,3,10);
			largomensaje = atoi(sizemensaje);


			char *mensajeEntero = malloc(largomensaje);
			recibir(sockKerCPU,&controlador,mensajeEntero,largomensaje);
			pcb = deserializarPCB_KerCPU(mensajeEntero);

			char *logeo1 = string_from_format("CASO N° 7: iniciar procesamiento de PCB-PID: %d",pcb->PID);
			escribir_log(logeo1,1);
			free(logeo1);

			procesar();

			free(mensajeEntero);
			free(sizemensaje);

			break;
		case 21:
			if(con_PCB == 0){

			}else{

				escribir_log("CASE N°21: devolver cpu por error",1);
				accion_siguiente = FINALIZAR_POR_ERROR;

				char *cod_error= string_substring(buff,13,4);
				int codigo_error= (-1)*(atoi(cod_error));

				pcb->exit_code = codigo_error;

				free(cod_error);

			}

			break;

		default:
			escribir_log("CASE DEFAULT: error - CPU desconoce ese mensaje",2);
			break;

		}

		free(idmensaje);

		switch(accion_siguiente){
		int size =0;
		int controlador=0;
		char *pcb_serializado;
		char *mensaje;

		case FINALIZAR_PROGRAMA:
			escribir_log("FINALIZO CORRECTAMENTE EL PROGRAMA... devolviendo pcb...",1);
			pcb->exit_code = 0;
			pcb_serializado  = serializarPCB_CPUKer(pcb,&size);
			mensaje = mensaje_pcb("P13",pcb_serializado,size);

			enviar(sockKerCPU,mensaje,&controlador,size+13);

			free(pcb_serializado);
			free(mensaje);
			accion_siguiente = CONTINUAR;
			break;

		case FINALIZAR_POR_QUANTUM:
			escribir_log("FINALIZO QUANTUM DEL PROGRAMA... devolviendo pcb...",1);

			pcb_serializado  = serializarPCB_CPUKer(pcb,&size);
			mensaje = mensaje_pcb("P12",pcb_serializado,size);

			enviar(sockKerCPU,mensaje,&controlador,size+13);

			free(pcb_serializado);
			free(mensaje);
			accion_siguiente = CONTINUAR;
			break;

		case FINALIZAR_POR_ERROR :

			escribir_log("FINALIZO PROGRAMA ERRONEAMENTE... devolviendo pcb ...",1);

			pcb_serializado  = serializarPCB_CPUKer(pcb,&size);
			mensaje = mensaje_pcb("P13",pcb_serializado,size);

			enviar(sockKerCPU,mensaje,&controlador,size+13);

			free(pcb_serializado);
			free(mensaje);
			accion_siguiente = CONTINUAR;
			break;

		case BLOQUEAR_PROCESO:
			escribir_log("PROGRAMA BLOQUEADO ... devolviendo pcb ...",1);
			pcb_serializado  = serializarPCB_CPUKer(pcb,&size);
			mensaje = mensaje_pcb("P16",pcb_serializado,size);

			enviar(sockKerCPU,mensaje,&controlador,size+13);

			free(pcb_serializado);
			free(mensaje);
			accion_siguiente = CONTINUAR;
			break;

		case CONTINUAR:
			escribir_log("CONTINUAR CON MI TRABAJO DE CPU",1);
			accion_siguiente = CONTINUAR;
		}

		if(ABORTAR == 1){

			escribir_log("FINALICE EL PROGRAMA ACTUAL PORQUE ME MANDARON A DESCONECTARME/MORIR... devolviendo pcb",1);
			chau = 1;

			char *mensaje = strdup("P190000000000");
			enviar(sockKerCPU,mensaje,&controlador,13);
			free(mensaje);

		}

		fin2: printf("\nfin\n");


		if(con_PCB==1) {
			liberar_pcb();
		}

	}

	free(buff);
	fin:

	//free(programa);
	free(ipK);
	free(ipM);
	liberar_log();

	return EXIT_SUCCESS;
}

void leerArchivoConfiguracion(char* argv)
{

	t_config *configuracion;
	configuracion = config_create(argv);
	if(config_has_property(configuracion,"PUERTO_KERNEL")&&
			config_has_property(configuracion,"PUERTO_MEMORIA")&&
			config_has_property(configuracion,"IP_KERNEL")&&
			config_has_property(configuracion,"IP_MEMORIA")){
		puertoK = config_get_int_value(configuracion, "PUERTO_KERNEL");
		puertoM = config_get_int_value(configuracion, "PUERTO_MEMORIA");
		ipK = strdup(config_get_string_value(configuracion, "IP_KERNEL"));
		ipM = strdup(config_get_string_value(configuracion, "IP_MEMORIA"));

		char * aux = string_from_format("archivo de configiguracion leido \n PUERTO_KERNEL:%d \n PUERTO_MEMORIA:%d \n IP_KERNEL:%s \n IP_MEMORIA:%s",puertoK,puertoM,ipK,ipM);
		escribir_log(aux,1);
		free(aux);
	}else {
		escribir_log("archivo de configiguracion incorrecto",2);
	}
	config_destroy(configuracion);
}
void procesar(){
	if(strncmp(pcb->algoritmo,"RR",2) == 0){
		//PROCESAR SEGUN QUANTUM/QUANTUM_SLEEP EL PCB
		char *alogge= string_from_format("Se procesa ROUND-ROBIN - Quantum:%d",pcb->quantum);
		escribir_log(alogge,1);
		free(alogge);

		int instrucciones_realizadas=0;
		accion_siguiente = CONTINUAR;
		while(instrucciones_realizadas < pcb->quantum && accion_siguiente != FINALIZAR_POR_ERROR && accion_siguiente != FINALIZAR_PROGRAMA && accion_siguiente != BLOQUEAR_PROCESO ){

			char * linea= pedir_linea_memoria();

			escribir_log(linea,1);
			analizadorLinea(linea,&funcionesTodaviaSirve,&funcionesKernelTodaviaSirve);
			free(linea);

			instrucciones_realizadas ++;
			if(salto_linea == SALTO_LINEA){
				salto_linea = 0;
			}else{
				pcb->PC++;
			}

			int controlador = 0;

			char *mensaje = malloc(17); memset(mensaje,'0',17);
			recibir_no_bloqueante(sockKerCPU,&controlador,mensaje,17);
			if(controlador == 1){
				char *cod = string_substring(mensaje,0,3);
				if(strncmp(cod,"K21",3) == 0){
					char *str_cod = string_substring(mensaje,13,4);
					int cod_error = (-1)* (atoi(str_cod));
					pcb->exit_code = cod_error;
					escribir_log("LOGEO CASO 21,DEVUELVO POR ERROR",1);
					fifo=FINALIZAR_POR_ERROR;
					accion_siguiente = FINALIZAR_POR_ERROR;
					free(str_cod);
				}
				free(cod);
			}
			free(mensaje);

			sleep(pcb->quantum_sleep/1000);
		}
		if((instrucciones_realizadas == pcb->quantum && accion_siguiente != FINALIZAR_PROGRAMA) || (instrucciones_realizadas == pcb->quantum && accion_siguiente != FINALIZAR_POR_ERROR)|| (instrucciones_realizadas == pcb->quantum && accion_siguiente != BLOQUEAR_PROCESO) ){
			accion_siguiente = FINALIZAR_POR_QUANTUM;
		}

	}else if(strncmp(pcb->algoritmo,"FF",2) == 0){
		escribir_log("Se procesa FIFO",1);
		// PROCESAR SEGUN FIFO
		fifo = CONTINUAR;
		while(fifo != FINALIZAR_PROGRAMA && fifo != FINALIZAR_POR_ERROR && fifo != BLOQUEAR_PROCESO){

			char *linea = pedir_linea_memoria();
			escribir_log("Linea a ejecutar:",1);
			escribir_log(linea,1);
			analizadorLinea(linea,&funcionesTodaviaSirve,&funcionesKernelTodaviaSirve);
			free(linea);

			if(salto_linea == SALTO_LINEA){
				salto_linea = 0;
			}else{
				pcb->PC++;
			}
			int controlador = 0;

			char *mensaje = malloc(17); memset(mensaje,'0',17);
			recibir_no_bloqueante(sockKerCPU,&controlador,mensaje,17);
			if(controlador == 1){
				char *cod = string_substring(mensaje,0,3);
				if(strncmp(cod,"K21",3) == 0){
					char *str_cod = string_substring(mensaje,13,4);
					int cod_error = (-1)* (atoi(str_cod));
					pcb->exit_code = cod_error;
					escribir_log("LOGEO CASO 21,DEVUELVO POR ERROR",1);
					fifo=FINALIZAR_POR_ERROR;
					accion_siguiente = FINALIZAR_POR_ERROR;
					free(str_cod);
				}
				free(cod);
			}
			free(mensaje);
			sleep(pcb->quantum_sleep/1000);
		}
	}
}
int conexion_Kernel(int puertoK, char* ipK) {
	int controladorConexion = 0;
	sockKerCPU = iniciar_socket_cliente(ipK, puertoK, &controladorConexion);
	if (controladorConexion == 0) {
		escribir_log("Exitos conectandose al Kernel, falta realizar handshake", 1);
	} else {
		escribir_log(string_itoa(controladorConexion),2);
	}
	printf("Socket para Kernel %d\n",sockKerCPU);
	int resultado = handshakeKernel(sockKerCPU);
	return resultado;
}
int conexion_Memoria(int puerto, char* ip) {
	int controladorConexion = 0;
	sockMemCPU = iniciar_socket_cliente(ip, puerto, &controladorConexion);
	if (controladorConexion == 0) {
		escribir_log("Exitos conectandose a Memoria", 1);
	} else {
		escribir_log(string_itoa(controladorConexion),2);
		return -1;
	}
	return handshakeMemoria(sockMemCPU);
}
char* pedir_linea_memoria(){
	int size = 0;
	int controlador = 0;
	int offset =pcb->in_cod[pcb->PC].offset_inicio;
	int largo = pcb->in_cod[pcb->PC].offset_fin;
	char *linea;

	if (linea_esta_dividida(calcular_offset_respecto_pagina(offset),largo) == true){

		int nuevo_largo1 = tam_pagina_memoria -  calcular_offset_respecto_pagina(offset);
		int nuevo_largo2 = largo - nuevo_largo1;
		char *primerostrece = malloc(13); memset(primerostrece,'\0',13);
		char *cantidad_linea;
		int largo_linea;
		char *primera_parte = malloc(nuevo_largo1+1); memset(primera_parte,'\0',nuevo_largo1+1);
		char *segunda_parte = malloc(nuevo_largo2+1); memset(segunda_parte,'\0',nuevo_largo2+1);

		char *mensaje = mensaje_leer_memoria(pcb->PID,offset,0,nuevo_largo1,&size);
		enviar(sockMemCPU,mensaje,&controlador,size);

		recibir(sockMemCPU,&controlador,primerostrece,13);
		cantidad_linea = string_substring(primerostrece,3,10);
		largo_linea = atoi(cantidad_linea);
		free(cantidad_linea);

		recibir(sockMemCPU,&controlador,primera_parte,largo_linea);

		memset(primerostrece,'\0',13);
		char *mensaje2 = mensaje_leer_memoria(pcb->PID,offset+nuevo_largo1,0,nuevo_largo2,&size);
		enviar(sockMemCPU,mensaje2,&controlador,size);

		recibir(sockMemCPU,&controlador,primerostrece,13);
		cantidad_linea = string_substring(primerostrece,3,10);
		largo_linea = atoi(cantidad_linea);
		free(cantidad_linea);

		recibir(sockMemCPU,&controlador,segunda_parte,largo_linea);

		string_trim(&primera_parte);
		string_trim(&segunda_parte);

		linea = string_from_format("%s%s",primera_parte,segunda_parte);

		free(primera_parte);
		free(segunda_parte);
		free(primerostrece);

	}else{

		char *mensaje = mensaje_leer_memoria(pcb->PID,offset,0,largo,&size);
		enviar(sockMemCPU,mensaje,&controlador,size);


		char *primerosbytes = malloc(13); memset(primerosbytes,'\0',13);
		recibir(sockMemCPU,&controlador,primerosbytes,13);

		char *cantidad = string_substring(primerosbytes,3,10);
		int largo_linea = atoi(cantidad);

		char *respuesta = malloc(largo_linea +1); memset(respuesta,'\0',largo_linea+ 1);
		recibir(sockMemCPU,&controlador,respuesta,largo_linea);


		linea = strdup(respuesta);

		free(mensaje);
		free(primerosbytes);
		free(cantidad);
		free(respuesta);
	}

	char * aux = string_from_format("Linea de offset :%d - longitud: %d",pcb->in_cod[pcb->PC].offset_inicio,pcb->in_cod[pcb->PC].offset_fin);
	escribir_log(aux,1);
	free(aux);

	return linea;
}

void finalizar_por_senial(int sig){

	signal(sig, SIG_IGN);
	if(con_PCB == 0){
		escribir_log("Señal recibida, pero no estaba ejecutando  , me voy de todos modos",1);
		escribir_log("      ¯_(ツ)_/¯   ",1);

		free(ipK);
		free(ipM);
		liberar_log();
		exit(0);

	}else{
		escribir_log("Señal recibida, morire pero no sin finalizar mis tareas ",1);
		ABORTAR = 1;
	}

}

void liberar_pcb(){

	escribir_log("LIBERANDO PCB",1);
	free(pcb->algoritmo);
	free(pcb->in_et);
	free(pcb->in_cod);

	list_destroy_and_destroy_elements(pcb->in_stack,(void *)stack_destroy);
	dictionary_destroy(pcb->dicc_et);

	free(pcb);

	con_PCB = 0;
}
/*
char *pedir_linea_memoria2(){
	char *linea = string_substring(programa,pcb->in_cod[pcb->PC].offset_inicio,pcb->in_cod[pcb->PC].offset_fin);
	return linea;
}*/
/*
void iniciar_pcb_falsa(){

	pcb= malloc(sizeof(t_PCB_CPU));
	pcb->PC =0;
	pcb->PID =1;
	pcb->SP = 0;
	pcb->cant_pag = 1;
	pcb->exit_code = 0;
	pcb->in_cod = armarIndiceCodigo(programa);
	pcb->in_et = armarIndiceEtiquetas(programa);
	pcb->in_stack = armarIndiceStack(programa);
	pcb->dicc_et = armarDiccionarioEtiquetas(pcb->in_et);
	pcb->algoritmo = strdup("FF");
}
 */
