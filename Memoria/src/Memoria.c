/*
 * Memoria2.c
 *
 *  Created on: 14/7/2017
 *      Author: utnso
 */

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "gestionar_procesos.h"
#include "consola_memoria.h"
#include "manejo_errores.h"
#include "estructuras.h"
#include "mensaje.h"
#include "socket.h"
#include "hash.h"
#include "log.h"



//Variables Hilos x Pedido
int cliente[20];
pthread_t hiloEsperarMensaje[20];

// Variables Globales
int hilo;
int tamanioMarco;
int cantMarcos;
int retardo;
int socketServerMemoria;
int controlador;
int STACK_SIZE;
int entradasCache;
int entradasProcesoCache;
int cantMarcosTablaPag;
int tamTablaPaginas;
int ultimo_frame_libre_asignado;

pthread_mutex_t mutex_memoria;
pthread_mutex_t mutex_cache;

t_tablaPagina *tablaPaginas;
t_memoria *data_Memoria;
t_log *log_;
t_LRU_cache *Cache_LRU;
t_cache *Cache;
t_dictionary** colisiones;
t_list* procesos;

char *Memoria;

void leerArchivoConfig(char* rutaArchivoConfig);
void inicializar_variables();
void esperar_mensaje(void *i);
void inicializarPrograma (int pid, int paginasRequeridas);
void mostrar_memoria (void);
void MostrarCache(void);
void incrementarLRU(int posInicializar);
void asignarPaginasProceso (int pid,int cantPaginasAgrabar);
void cargarTablaPaginasEnMemoria();
int liberar_paginas_pid(int pid);
int liberar_una_pagina(int pid,int pag);
//void cargarCodigoEnMemoria(char * datos, char * Memoria ,t_list * frames_pedidos, int cantPaginasAgrabar);
//void liberar_valor(char *val);

int cantidadDePaginas(int tamanioBytes);
int posFrameEnMemoria(int nroFrame);
int maxPaginaPid(int p_pid);
int buscarPosCache(int pid, int pag);
int posPaginaSolicitada(int pid,int paginaSolicitada);
int cantMarcosLibre();
int posLibreMemoria(int pos);

bool f_mayor(int a, int b);

char * solicitarBytes(int pid, int pag, int offset, int tam);
char* almacenarBytes(int pid, int pag, int offset, int tam, char * buf);

int cantPidEnCache(int pid);
int sustituirLRU(int pid, int pag);
int posMaxLRU(void);
int maxLRUproc(int pid);
int sustituirLRUproc(int pid, int pag);

t_tablaPagina* crearEstrucTablaPag(void);
t_LRU_cache* crearCacheLRU(void);
t_cache* crearMemoCache(void);



int main(int argc, char *argv[])
{
	pthread_t hiloConsolaMemoria;

	crear_archivo_log("/home/utnso/MEMORIA-DEBUGGING-LOG.txt");
	leerArchivoConfig(argv[1]);

	inicializar_variables();

	tablaPaginas = crearEstrucTablaPag();

	Memoria = malloc(cantMarcos * tamanioMarco);
	memset(Memoria,' ',cantMarcos * tamanioMarco);


	if(entradasCache != 0){
		Cache = crearMemoCache();
		Cache_LRU= crearCacheLRU();
	}
	/*	{		escribir_log("Imprimiendo Caché");

		int d;
		for(d=0;d<entradasCache;d++){

			char *aux_log = string_from_format("\n %d | %d | %s \n",Cache[d].pid,Cache[d].pag,Cache[d].dataFrame);
			escribir_log(aux_log);
			free(aux_log);
		}
	}
*/
	escribir_log_con_numero("Tamaño de Tabla de paginas",tamTablaPaginas);
	escribir_log_con_numero("Tamaño Total de Memoria",tamanioMarco * cantMarcos);

	cargarTablaPaginasEnMemoria();
/*
 	escribir_log("Imprimiendo Tabla Paginas");
	int it;
	for(it=0;it<cantMarcos;it++){
		char *aux_log = string_from_format("-%d- %d | %d | %d \n",it,tablaPaginas[it].estado,tablaPaginas[it].pid,tablaPaginas[it].pag);
		escribir_log(aux_log);
		free(aux_log);
	}
*/

	socketServerMemoria = iniciar_socket_server(data_Memoria->IP,data_Memoria->PUERTO,&controlador);

	char *log_aux = string_from_format("Se inicia Servidor de Memoria en Socket: %d ",socketServerMemoria);
	escribir_log(log_aux);
	free(log_aux);


	pthread_create(&hiloConsolaMemoria, NULL, (void*)hilo_consola_memoria, NULL);

	hilo = 0;
	while (hilo < 20) {

		cliente[hilo] = escuchar_conexiones(socketServerMemoria,&controlador);
		char *log_aux= string_from_format("Nueva Conexión entrante: %d ",cliente[hilo]);
		escribir_log(log_aux);
		free(log_aux);

		// Realizar HS y enviar Tamaño MARCO.
		char *mensaje_recibido = recibir(cliente[hilo], &controlador);
		char *header = get_header(mensaje_recibido);


		if(strcmp(header,"K")==0 || strcmp(header,"P")==0 )
		{
			//Cliente VALIDO
			char *str_marcos_size = string_itoa(tamanioMarco);
			char *HS_OK = armar_mensaje("M00",str_marcos_size); // M|00|0000000003|128|
			enviar(cliente[hilo],HS_OK, &controlador);
			free(str_marcos_size);
			free(HS_OK);
		}
		else
		{
			char *aux_log = string_from_format("Se cierra la conexión con el Cliente luego del HandShake - Socket: %d ",cliente[hilo]);
			escribir_log(aux_log);
			free(aux_log);
			cerrar_conexion(cliente[hilo]);
			goto salir_handshake;
		}

		//Si es Cliente valido entra a interactuar con la MEMORIA. // Hilo
		free(mensaje_recibido);
		free(header);

		pthread_create(&hiloEsperarMensaje[hilo], NULL, (void*) esperar_mensaje,(void *) cliente[hilo]);
		//pthread_join(hiloEsperarMensaje[hilo],NULL);

		log_aux= string_from_format("El Cliente %d tiene Hilo nro :%d ",cliente[hilo],hilo);
		escribir_log(log_aux);
		free(log_aux);

		// RECIBIR
		salir_handshake:
		hilo++;
	}

	free(colisiones);
	free(tablaPaginas);
	free(Memoria);
	if(entradasCache != 0){
		int i;
		for(i=0;i<entradasCache;i++){
				free(Cache[i].dataFrame);
		}
		free(Cache);
		free(Cache_LRU);
	}
	free(data_Memoria->IP);
	free(data_Memoria);

	pthread_mutex_destroy(&mutex_cache);
	pthread_mutex_destroy(&mutex_memoria);

	return EXIT_SUCCESS;
}

void leerArchivoConfig(char* rutaArchivoConfig)
{
	data_Memoria = malloc(sizeof(t_memoria));
	data_Memoria->IP = strdup("");

	t_config *configuracion = config_create(rutaArchivoConfig);
	string_append(&data_Memoria->IP,config_get_string_value(configuracion,"IP"));
	data_Memoria->PUERTO = config_get_int_value(configuracion,"PUERTO");
	data_Memoria->MARCOS = config_get_int_value(configuracion,"MARCOS");
	data_Memoria->MARCO_SIZE = config_get_int_value(configuracion,"MARCO_SIZE");
	data_Memoria->ENTRADAS_CACHE = config_get_int_value(configuracion,"ENTRADAS_CACHE");
	data_Memoria->CACHE_X_PROC = config_get_int_value(configuracion,"CACHE_X_PROC");
	data_Memoria->REEMPLAZO_CACHE = config_get_int_value(configuracion,"REEMPLAZO_CACHE");
	data_Memoria->RETARDO_MEMORIA = config_get_int_value(configuracion,"RETARDO_MEMORIA");

	config_destroy(configuracion);
}

void inicializar_variables(){

	tamanioMarco = data_Memoria->MARCO_SIZE;
	cantMarcos = data_Memoria->MARCOS;
	entradasCache = data_Memoria->ENTRADAS_CACHE;
	entradasProcesoCache = data_Memoria->CACHE_X_PROC;
	tamTablaPaginas = sizeof(t_tablaPagina) * cantMarcos;
	cantMarcosTablaPag = cantidadDePaginas(tamTablaPaginas);
	ultimo_frame_libre_asignado = cantMarcosTablaPag;
	inicializar_array_colisiones();
	retardo = data_Memoria->RETARDO_MEMORIA;
	procesos = list_create();
	pthread_mutex_init(&mutex_memoria,NULL);
	pthread_mutex_init(&mutex_cache,NULL);


}

int cantidadDePaginas(int tamanioBytes)
{
	double Bloques= -1;
	if ((double)tamanioBytes/tamanioMarco - (int)(tamanioBytes/tamanioMarco) > 0){
		Bloques = ((tamanioBytes/tamanioMarco) +1);
	}else{
		Bloques = (int)(tamanioBytes/tamanioMarco);
	}
	return (int)Bloques;

}

t_tablaPagina* crearEstrucTablaPag(void){

	t_tablaPagina *estructuraTablaPaginas = malloc(sizeof(t_tablaPagina)*cantMarcos);
	int i;
	for(i=0;i<cantMarcos;i++){
		estructuraTablaPaginas[i].estado=0;
		estructuraTablaPaginas[i].pag=-1;
		estructuraTablaPaginas[i].pid=-1;
	}
	return estructuraTablaPaginas;
}

t_cache* crearMemoCache(void){

	t_cache* MemoCache=malloc(sizeof(t_cache)*entradasCache);
	int i;
	for(i=0;i<entradasCache;i++){
		MemoCache[i].pag=-1;
		MemoCache[i].pid=-1;
		MemoCache[i].dataFrame = NULL;
	}
	return MemoCache;
}

t_LRU_cache *crearCacheLRU(void){

	t_LRU_cache* adminLRU=malloc(sizeof(t_LRU_cache)*entradasCache);
	int i;
	for(i=0;i<entradasCache;i++){
		adminLRU[i].pag=-1;
		adminLRU[i].pid=-1;
		adminLRU[i].LRU=-1;
	}
	return adminLRU;
}
void cargarTablaPaginasEnMemoria(){

	int i_tp;
	int inicio=0;
	for(i_tp=0;i_tp<cantMarcosTablaPag;i_tp++){
		tablaPaginas[i_tp].estado = 1;
		tablaPaginas[i_tp].pag= inicio;
		inicio ++;
	}

	memcpy(Memoria,tablaPaginas,tamTablaPaginas);
}

void esperar_mensaje(void *i) {

	int cliente = (int) i;
	int chau = 0;
	while(chau != 1){

		char *mensRec = malloc(1024);
		memset(mensRec,'\0', 1024);

		mensRec = recibir(cliente, &controlador);

		if(controlador != 0)
		{
			cerrar_conexion(cliente);
			chau =1;
			hilo=30;
			goto chau;
		}

		char *header=get_header(mensRec);
		char *cod = get_codigo(mensRec);
		int codigo = atoi(cod);

		escribir_log_compuesto(" HEADER: ",header);
		escribir_log_con_numero(" CODIGO: ",codigo);

		if (!strcmp(header,"K"))
		{
			switch (codigo)
			{
			case 06: //INICIALIZAR PROGRAMA
			{
				//K|17|PPPP|XXXX|YYYY|
				//PPPP: Pid
				//XXXX: CantidadPaginasCodigo
				//YYYY: CantidadPaginasSTACK
				char *log_aux3 = string_from_format("K06 - Inicializar Programa : %d",cliente);
				escribir_log(log_aux3);
				free(log_aux3);

				escribir_log_compuesto("Mensaje Recibido de Inicializar Programa: \n ",mensRec);

				char *str_pid = string_substring(mensRec, 3, 4);
				int procPid= atoi(str_pid);
				char *paginasCodigo = string_substring(mensRec, 7, 4);
				int pagsCodigo = atoi(paginasCodigo);
				char *paginasSTACK = string_substring(mensRec, 11, 4);
				STACK_SIZE = atoi(paginasSTACK);

				escribir_log_con_numero("PID RECIBIDO:",procPid);
				escribir_log_con_numero("TAMAÑO PAGINAS CODIGO:",pagsCodigo);
				escribir_log_con_numero("TAÑANO PAGINAS STACK:",STACK_SIZE);

				int libres = cantMarcosLibre();

				if ( libres >= (pagsCodigo + STACK_SIZE))
				{

					inicializarPrograma (procPid,pagsCodigo);
					crear_proceso(procPid,pagsCodigo);
					inicializarPrograma (procPid,STACK_SIZE);
					actualizar_paginas(procPid,STACK_SIZE);
					actualizar_maxnro_pagina(procPid,STACK_SIZE);

					char* r_OK = armar_mensaje("M02","");
					enviar(cliente,r_OK, &controlador);
					free(r_OK);

					char *log_aux = string_from_format("K06: Se Inicializo el PROGRAMA CORRECTAMENTE | Pid: %d",procPid);
					escribir_log(log_aux);
					free(log_aux);


				} else
				{

					char* r_NOK = armar_mensaje("M03","");//M03
					enviar(cliente,r_NOK, &controlador);
					free(r_NOK);

					char *log_aux = string_from_format("K06: No se puede inicializar el proceso: %d",cliente);
					escribir_log(log_aux);
					free(log_aux);
				}



				escribir_log("Imprimiendo Tabla Paginas");
				int it;
				for(it=0;it<cantMarcos;it++){
					char *aux_log = string_from_format("-%d- %d | %d | %d",it,tablaPaginas[it].estado,tablaPaginas[it].pid,tablaPaginas[it].pag);
					escribir_log(aux_log);
					free(aux_log);
				}

				free(str_pid);
				free(paginasCodigo);
				free(paginasSTACK);

			}
			break;

			/*case 200: //K|18|CANTIDADBYTES|CODIGO|PID
			{
				char *escribir =string_from_format("K20 - Guardando codigo: %d ",cliente);
				escribir_log(escribir);
				free(escribir);


				char *tam = string_substring(mensRec, 3, 10);
				int tamanio = atoi(tam);
				free(tam);

				char * codigo = string_substring(mensRec, 13, tamanio);
				int pagsCod = cantidadDePaginas(tamanio);

				char *procP = string_substring(mensRec, 13+tamanio, 4);
				int procPid = atoi(procP);
				free(procP);

				escribir_log("\n ----------------------------- \n");
				escribir_log(mensRec);
				escribir_log_con_numero("PID: ",procPid);
				escribir_log_con_numero("PCOD: ",pagsCod);
				escribir_log_compuesto("CODIGO: ", codigo);
				escribir_log("\n ----------------------------- \n");
				//Meter funciones de guardar codigo
				escribir_log("\n Accediendo a Memoria Principal ... \n");
				sleep(retardo/1000);
				pthread_mutex_lock(&mutex_memoria);
				int i;
				for (i=0;i<pagsCod; i++)
				{
					int frame_pos = posPaginaSolicitada(procPid,i);
					escribir_log_con_numero("\n POS: \n",frame_pos);
					int pos = posFrameEnMemoria(frame_pos);
					memcpy(Memoria+pos,codigo+(i*tamanioMarco),tamanioMarco);
					//actualizarCache

				}
				pthread_mutex_unlock(&mutex_memoria);

				free(codigo);

				mostrar_memoria ();

			}
			break;
			 */
			case 19:// K1900000001
			{
				//K|19|PPPP|XXXX.
				//PPPP: PID.
				//XXXX: CantidadPaginas.
				char *log_aux4 = string_from_format("K19 - Solicitar Paginas HEAP: %d",cliente);
				escribir_log(log_aux4);
				free(log_aux4);

				escribir_log_compuesto("\n MENSAJE RECIBIDO:   ",mensRec);
				char *str_pid = string_substring(mensRec, 3, 4);
				int procPid= atoi(str_pid);
				char *paginasHEAP = string_substring(mensRec, 7, 4);
				int pagsHEAP = atoi(paginasHEAP);

				escribir_log_con_numero("PID RECIBIDO: ",procPid);
				escribir_log_con_numero("CANTIDAD PAGINAS PARA HEAP: ",pagsHEAP);

				int libres = cantMarcosLibre();

				if ( libres >= (pagsHEAP))
				{
					asignarPaginasProceso (procPid,pagsHEAP);
					actualizar_paginas(procPid,pagsHEAP);
					actualizar_maxnro_pagina(procPid,pagsHEAP);
					char* r_OK = armar_mensaje("M02","");
					enviar(cliente,r_OK, &controlador);
					free(r_OK);

					char *logi=string_from_format("K19- Se pudo guardar el CODIGO para HEAP -: %d",cliente);
					escribir_log(logi);
					free(logi);

				} else
				{

					char* r_NOK = armar_mensaje("M03","");//M03
					enviar(cliente,r_NOK, &controlador);
					free(r_NOK);

					char *logi = string_from_format("K19- NO se pudo guardar el CODIGO para HEAP: %d",cliente);
					escribir_log(logi);
					free(logi);
				}



				escribir_log("Imprimiendo Tabla Paginas");
				int it;
				for(it=0;it<cantMarcos;it++){
					char *aux_log = string_from_format("-%d- %d | %d | %d ",it,tablaPaginas[it].estado,tablaPaginas[it].pid,tablaPaginas[it].pag);
					escribir_log(aux_log);
					free(aux_log);
				}

				free(str_pid);
				free(paginasHEAP);


			}
			break;
			case 25:
			{
				char *log_aux6 = string_from_format("K25 - Liberar Proceso: %d",cliente);
				escribir_log(log_aux6);
				free(log_aux6);

				char *procpid = string_substring(mensRec,3,4);
				int pid = atoi(procpid);
				pthread_mutex_lock(&mutex_cache);
				int retonro = liberar_paginas_pid(pid);
				pthread_mutex_unlock(&mutex_cache);
				free(procpid);

				char *mensaje;
				if(retonro == 0){
					mensaje = armar_mensaje("M02","");
				}else{
					mensaje = armar_mensaje("M03","");
				}
				enviar(cliente,mensaje,&controlador);
				free(mensaje);

				escribir_log_con_numero("FINALIZAR PROGRAMA REALIZADO POR PID: ",pid);
			}
			break;
			case 24: //K24|PID|PAG
			{

				char *log_aux7 = string_from_format("K24-Liberar Pagina de un proceso: %d",cliente);
				escribir_log(log_aux7);
				free(log_aux7);

				char *ppid = string_substring(mensRec,3,4);
				int pid = atoi(ppid);
				char *ppag = string_substring(mensRec,7,4);
				int pag = atoi(ppag);

				free(ppid);free(ppag);
				pthread_mutex_lock(&mutex_cache);
				int retorno = liberar_una_pagina(pid,pag);
				pthread_mutex_unlock(&mutex_cache);
				actualizar_paginas(pid,-1);

				char *mensaje;
				if(retorno == 0){
					mensaje = armar_mensaje("M02","");
				}else{
					mensaje = armar_mensaje("M03","");
				}
				enviar(cliente,mensaje,&controlador);
				free(mensaje);

				escribir_log_con_numero("\nEliminando la PAGINA:",pag);
				escribir_log_con_numero("PID:",pid);


			}
			break;
			case 94: //Solicitar Bytes de Memoria
			{
				char *log_aux8 = string_from_format("K94 KERNEL Solicita Bytes: %d",cliente);
				escribir_log(log_aux8);
				free(log_aux8);
				char *ppid = string_substring(mensRec, 3, 4);
				int pid = atoi(ppid);
				char *ppag = string_substring(mensRec, 7, 4);
				int pag = atoi(ppag);
				char *poffset = string_substring(mensRec, 11, 4);
				int offset= atoi(poffset);
				char *ptam = string_substring(mensRec, 15, 4);
				int tam = atoi(ptam);

				char *logi=string_from_format("Solicitar Bytes \nPID:%d \nPAG:%d \nOFFSET:%d \nTAM:%d\n",pid, pag, offset,tam);
				escribir_log(logi);
				free(logi);

				pthread_mutex_lock(&mutex_cache);
				char * Buffer = solicitarBytes(pid, pag, offset, tam);
				pthread_mutex_unlock(&mutex_cache);


				char *buffer_aux=string_substring(Buffer,0,tam);


				if ( strncmp(Buffer,"M03",3) == 0)
				{
					char *RTA_OK = armar_mensaje("M03",""); // M|03|0000000000
					escribir_log_compuesto("\n Solicitar Bytes - ERROR: PID|PAG invalida \n", RTA_OK);
					enviar(cliente,RTA_OK, &controlador);
					free(RTA_OK);
				} else
				{
					char *RTA_OK = armar_mensaje("M08",buffer_aux);
					escribir_log_compuesto("\n BUFFER RETORNO SOLICITAR BYTES \n", RTA_OK);
					enviar(cliente,RTA_OK, &controlador);
					free(RTA_OK);
				}
				free(ppid);
				free(ppag);
				free(ptam);
				free(poffset);
				free(Buffer);
				free(buffer_aux);

			}
			break;
			case 90: //K90
				{
				char *log_aux9 = string_from_format("K90 - Almacenar Bytes: %d",cliente);
				escribir_log(log_aux9);
				free(log_aux9);


				char *ppid = string_substring(mensRec, 3, 4);
				int pid = atoi(ppid);
				char *ppag = string_substring(mensRec, 7, 4);
				int pag = atoi(ppag);
				char *poffset = string_substring(mensRec, 11, 4);
				int offset= atoi(poffset);
				char *ptam = string_substring(mensRec, 15, 4);
				int tam = atoi(ptam);
				char *pbuffer = string_substring(mensRec, 19, tam);

				char *logi = string_from_format("Almacenar Bytes \nPID:%d \nPAG:%d \nOFFSET:%d \nTAM:%d\n BUFFER:%s \n",pid, pag, offset,tam,pbuffer);
				escribir_log(logi);
				free(logi);

				pthread_mutex_lock(&mutex_memoria);
				char* r_OK = almacenarBytes(pid, pag, offset, tam, pbuffer);
				pthread_mutex_unlock(&mutex_memoria);

				enviar(cliente,r_OK, &controlador);


				free(ppid);
				free(ppag);
				free(ptam);
				free(pbuffer);
				free(poffset);
				free(r_OK);
			}
			break;

			case 99: //Muestra MP
			{
				mostrar_memoria();
			}
			break;
			case 98: //Muestra CACHE
			{
				MostrarCache();
			}
			break;
			case 66:
				chau =1;
				hilo=60;
				break;
			}

			free(header);
		}else{
			switch (codigo)
			{
			case 01: //Solicitar Bytes de Memoria
			{
				char *log_aux1 = string_from_format("P01 - CPU Solicita Bytes: %d",cliente);
				escribir_log(log_aux1);
				free(log_aux1);
				char *ppid = string_substring(mensRec, 3, 4);
				int pid = atoi(ppid);
				char *ppag = string_substring(mensRec, 7, 4);
				int pag = atoi(ppag);
				char *poffset = string_substring(mensRec, 11, 4);
				int offset= atoi(poffset);
				char *ptam = string_substring(mensRec, 15, 4);
				int tam = atoi(ptam);

				char *logi=string_from_format("Solicitar Bytes \nPID:%d \nPAG:%d \nOFFSET:%d \nTAM:%d\n",pid, pag, offset,tam);
				escribir_log(logi);
				free(logi);

				pthread_mutex_lock(&mutex_cache);
				char * Buffer = solicitarBytes(pid, pag, offset, tam);
				pthread_mutex_unlock(&mutex_cache);

				char *buffer_aux=string_substring(Buffer,0,tam);

				if ( strncmp(buffer_aux,"M03",3) == 0)
				{
					char *RTA_OK = armar_mensaje("M03",""); // M|03|0000000000
					escribir_log_compuesto("\n Solicitar Bytes - ERROR: PID|PAG invalida \n", RTA_OK);
					enviar(cliente,RTA_OK, &controlador);
					free(RTA_OK);
				} else
				{
					char *RTA_OK = armar_mensaje_pcb("M08",buffer_aux,tam);
					escribir_log_compuesto("\n BUFFER RETORNO SOLICITAR BYTES \n", RTA_OK);
					//enviar(cliente,RTA_OK, &controlador);
					send(cliente,RTA_OK,tam+13, 0);
					free(RTA_OK);
				}
				free(ppid);
				free(ppag);
				free(ptam);
				free(poffset);
				free(Buffer);
				free(buffer_aux);

			}

			break;
			case 8: //K|18|PIDD|CANTIDADBYTES|CODIGO}{}
				{
				char *log_aux2 = string_from_format("P08-Almacenar Bytes: %d",cliente);
				escribir_log(log_aux2);
				free(log_aux2);


				char *ppid = string_substring(mensRec, 3, 4);
				int pid = atoi(ppid);
				char *ppag = string_substring(mensRec, 7, 4);
				int pag = atoi(ppag);
				char *poffset = string_substring(mensRec, 11, 4);
				int offset= atoi(poffset);
				char *ptam = string_substring(mensRec, 15, 4);
				int tam = atoi(ptam);
				char *pbuffer = string_substring(mensRec, 19, tam);

				char *logi = string_from_format("Almacenar Bytes \nPID:%d \nPAG:%d \nOFFSET:%d \nTAM:%d\n BUFFER:%s \n",pid, pag, offset,tam,pbuffer);
				escribir_log(logi);
				free(logi);
				pthread_mutex_lock(&mutex_memoria);
				char* r_OK = almacenarBytes(pid, pag, offset, tam, pbuffer);
				pthread_mutex_unlock(&mutex_memoria);
				enviar(cliente,r_OK, &controlador);


				free(ppid);
				free(ppag);
				free(ptam);
				free(pbuffer);
				free(poffset);
				free(r_OK);
			}
			break;
			case 66:
				chau =1;
				break;
			case 40:
				chau=1;
				escribir_log_con_numero("\n Se desconectó CPU: \n",cliente);
				close(cliente);
				break;
			default:
				chau=1;
				escribir_log_con_numero("\n Se desconectó CPU: \n",cliente);
				close(cliente);


			}
		}



		chau:
		free(mensRec);
		//printf("Ejecutada \n");
	}

}
int cantMarcosLibre(){
	int it;
	int a=0;
	for(it=cantMarcosTablaPag;it<cantMarcos;it++)
	{
		if ( tablaPaginas[it].estado == 0)
		{
			a++;
		}
	}
	escribir_log_con_numero("\n CANTIDAD MARCOS LIBRES: \n",a);

	return a;
}

void inicializarPrograma (int pid,int cantPaginasAgrabar)
{
	//Calcular posición frames libres más próximos para una cantidad solicitada.
	int maxPagPid2 = ultimoNumeroPagina(pid);
	int maxPagPid = maxPaginaPid(pid); //Funcion que me devuelva la máxima pagina de un PID.
	escribir_log_con_numero("MAX PAGINA COMO SIEMPRE",maxPagPid);
	escribir_log_con_numero("MAX PAGINA CON LISTA",maxPagPid2);
	int pos_hash;
	int pos_ReHash;
	int i;

	for (i = 0; i< cantPaginasAgrabar;i++)
	{
		int pag = maxPagPid +1;
		escribir_log("\n ANTES DE LA HASHH: \n");
		pos_hash = hash(pid, pag );
		escribir_log_con_numero("\n pos_has: \n",pos_hash);

		if (marco_ocupado(pos_hash) == false ){
			tablaPaginas[pos_hash].estado = 1;
			tablaPaginas[pos_hash].pid = pid;
			tablaPaginas[pos_hash].pag = maxPagPid+1;
			maxPagPid++;
		}
		else
		{
			pos_ReHash = reasignar_colision();
			ultimo_frame_libre_asignado = pos_ReHash;
			asentar_colision(pos_ReHash,pos_hash,pid, pag);
			tablaPaginas[pos_ReHash].estado = 1;
			tablaPaginas[pos_ReHash].pid = pid;
			tablaPaginas[pos_ReHash].pag = maxPagPid+1;
		//	printf("\n pos_RE_has: %d \n",pos_ReHash);
			maxPagPid++;
		}
	}
}

int posPaginaSolicitada(int pid,int paginaSolicitada)
{
	int hash_pos = hash(pid,paginaSolicitada);
	if (es_marco_correcto(pid, paginaSolicitada,hash_pos) == true ) {
		return hash_pos;
	}
	else {
		int re_hash_pos = buscar_marco_colision(pid,paginaSolicitada,hash_pos);
		return re_hash_pos;
	}
}
int posFrameEnMemoria(int nroFrame){
	int pos= (nroFrame* tamanioMarco)-1;
	return pos;
}

void mostrar_memoria (void)
{

	int a;

	escribir_log("\n ------------------------------------------- \n");
	escribir_log("\n IMPRIMIENDO MEMORIA \n");

	for (a = cantMarcosTablaPag; a<cantMarcos; a ++ )
	{
		int pos = posFrameEnMemoria(a);

		char * dataFrame = malloc (tamanioMarco); memset(dataFrame,'\0',tamanioMarco);
		memcpy(dataFrame,Memoria+pos,tamanioMarco);

		char * imp = string_from_format("|%s| - %d \n",dataFrame,a);
		escribir_log(imp);
		free(imp);
		free(dataFrame);
	}

	escribir_log("\n ------------------------------------------- \n");
	escribir_log("\n IMPRIMIENDO MEMORIA CACHÉ \n");
	int i;
	char * imp;
	for(i=0;i<entradasCache;i++){

	//	printf("\n%d|%d|%s \n",Cache[i].pid,Cache[i].pag,Cache[i].dataFrame);
		imp = string_from_format("\n%d|%d|%s \n",Cache[i].pid,Cache[i].pag,Cache[i].dataFrame);
		escribir_log(imp);
		free(imp);

	}


}

void asignarPaginasProceso (int pid,int cantPaginasAgrabar)
{
	//Calcular posición frames libres más próximos para una cantidad solicitada.
	int maxPagPid2 = ultimoNumeroPagina(pid);
	int maxPagPid = maxPaginaPid(pid); //Funcion que me devuelva la máxima pagina de un PID.
	escribir_log_con_numero("MAX PAGINA COMO SIEMPRE",maxPagPid);
	escribir_log_con_numero("MAX PAGINA CON LISTA",maxPagPid2); //Funcion que me devuelva la máxima pagina de un PID.

	int pos_hash;
	int pos_ReHash;
	int i;

	for (i = 0; i< cantPaginasAgrabar;i++)
	{
		int pag = maxPagPid +1;
		escribir_log("\n ANTES DE LA HASHH: \n");
		pos_hash= hash(pid, pag );
		escribir_log_con_numero("\n pos_has: ",pos_hash);

		if (marco_ocupado(pos_hash) == false ){
			tablaPaginas[pos_hash].estado = 1;
			tablaPaginas[pos_hash].pid = pid;
			tablaPaginas[pos_hash].pag = maxPagPid+1;
			maxPagPid++;
		}
		else
		{
			pos_ReHash = reasignar_colision();
			ultimo_frame_libre_asignado = pos_ReHash;
			asentar_colision(pos_ReHash,pos_hash,pid, pag);
			tablaPaginas[pos_ReHash].estado = 1;
			tablaPaginas[pos_ReHash].pid = pid;
			tablaPaginas[pos_ReHash].pag = maxPagPid+1;
			escribir_log_con_numero("\n pos_RE_has: ",pos_ReHash);
			maxPagPid++;
		}
	}
}

void MostrarCache(void){

	escribir_log("\n ------------------------------------------- \n");
	int i;
	escribir_log(" IMPRIMIENDO CACHE LRU");
	char *imp_lru;
	for(i=0;i<entradasCache;i++){


		imp_lru = string_from_format("\n%d|%d|%d \n",Cache_LRU[i].pid,Cache_LRU[i].pag,Cache_LRU[i].LRU);
		escribir_log(imp_lru);
		free(imp_lru);

	}
	escribir_log("\n ------------------------------------------- \n");
	escribir_log(" IMPRIMIENDO CACHE COMÚN");
	char *imp_cache;
	for(i=0;i<entradasCache;i++){


	imp_cache = string_from_format("\n%d|%d|%s \n",Cache[i].pid,Cache[i].pag,Cache[i].dataFrame);
		escribir_log(imp_cache);
		free(imp_cache);

	}
	escribir_log("\n ------------------------------------------- \n");

}

int maxPaginaPid(int p_pid)
{
	t_list* lt_pagsXpid= list_create();
	int primeraPosicion = 0;
	int z;
	int max=-1;
	for (z=cantMarcosTablaPag; z<cantMarcos; z++ )
	{
		if ( tablaPaginas[z].estado == 1 && tablaPaginas[z].pid == p_pid) {
			list_add(lt_pagsXpid,(void *)tablaPaginas[z].pag);
		}
	}
	list_sort(lt_pagsXpid,(void*) f_mayor);

	if ( list_size(lt_pagsXpid) != 0 ){
		max=(int)list_get(lt_pagsXpid,primeraPosicion);
	}

	return max;
}
bool f_mayor(int a, int b) {
	return a > b;
}

char * solicitarBytes(int pid, int pag, int offset, int tam)
{
	int frame_pos = posPaginaSolicitada(pid,pag);
	escribir_log_con_numero("Posicion del Frame en Memoria: ",frame_pos);

	if (frame_pos <= 0 ) { //No encontro la pagina.
		char* rta = strdup("M030000000000");
		return rta;
	}

	if(entradasCache != 0){ // CACHE HABILITADA
		int pEnCache = buscarPosCache(pid, pag);
		if (pEnCache == -1) //La pagina no se encuentra en CACHE
		{
			escribir_log("--------- Cache Miss - LA PAGINA SOLICITADA NO SE ENCUENTRA EN CACHE ------- ");
			escribir_log("---------  Accediendo a Memoria Principal: Lectura ... -----------");
			sleep(retardo/1000);
			int pos = posFrameEnMemoria(frame_pos);
			char * dataFrame = malloc (tam);
			memset(dataFrame,'\0',tam);
			memcpy(dataFrame,Memoria+pos+offset,tam);

			int entradasPid = cantPidEnCache(pid);

			if (entradasPid < entradasProcesoCache)
			{
				escribir_log("---------  Actualizando Memoria Cache: Algoritmo LRU ... -----------");
				int posReemplazoCache = sustituirLRU(pid,pag);
				char *frame = malloc(tamanioMarco);
				memset(frame,'\0',tamanioMarco);
				memcpy(frame,Memoria+pos,tamanioMarco);
				Cache[posReemplazoCache].pid=pid;
				Cache[posReemplazoCache].pag=pag;
				free(Cache[posReemplazoCache].dataFrame);
				Cache[posReemplazoCache].dataFrame = frame;


			} else
			{
				escribir_log("---------  Actualizando Memoria Cache: Algoritmo LRU ... -----------");
				int posReemplazoCache = sustituirLRUproc(pid,pag);
				char * frame = malloc(tamanioMarco);
				memset(frame,'\0',tamanioMarco);
				memcpy(frame,Memoria+pos,tamanioMarco);
				Cache[posReemplazoCache].pid=pid;
				Cache[posReemplazoCache].pag=pag;
				free(Cache[posReemplazoCache].dataFrame);
				Cache[posReemplazoCache].dataFrame = frame;

			}

			return dataFrame;
		}
		else //La pagina esta en CACHE
		{
			escribir_log("---------  Accediendo a Memoria Cache: Lectura ... -----------");
			char * dataFrame = malloc (tam);
			memset(dataFrame,'\0',tam);
			memcpy(dataFrame,Cache[pEnCache].dataFrame+offset, tam);
			incrementarLRU(pEnCache);
			Cache_LRU[pEnCache].LRU = 0;
			//++++++++++++++++++++//
			// UpdateLRU(pid,pag);//
			//++++++++++++++++++++//
			return dataFrame;

		}
	}else{ //CACHE DESHABILITADA
		escribir_log(" -----------  Accediendo a Memoria Principal: Lectura ... ------------ ");
		sleep(retardo/1000);
		int pos = posFrameEnMemoria(frame_pos);
		char *dataFrame = malloc (tam);
		memset(dataFrame,'\0',tam);
		memcpy(dataFrame,Memoria+pos+offset,tam);

		return dataFrame;
	}




}

int buscarPosCache(int pid, int pag)
{
	//devuelve la pos si no -1
	int posCache;
	int i;
	for(i=0;i<entradasCache;i++){
		if (Cache[i].pid == pid && Cache[i].pag == pag){
			posCache = i;
			return posCache;
		} else {
			posCache = -1;
		}
	}
	return posCache;
}

int cantPidEnCache(int pid){
	int it;
	int a=0;
	for(it=0;it<entradasCache;it++)
	{
		if ( Cache_LRU[it].pid== pid)
		{
			a++;
		}
	}
	escribir_log_con_numero("--------- Cantidad de un Pids en CACHE: ", a);
	return a;
}
int sustituirLRU(int pid, int pag)
{
int it;

for(it=0;it<entradasCache;it++)
{
	if ( Cache_LRU[it].LRU== -1)
		{
		incrementarLRU(it);
		Cache_LRU[it].pid=pid;
		Cache_LRU[it].pag=pag;
		Cache_LRU[it].LRU=0;
		return it;
		}

}

int reemplazoLRU = posMaxLRU();

for(it=0;it<entradasCache;it++)
{
	if (Cache_LRU[it].LRU== reemplazoLRU)
		{
		incrementarLRU(it);
		Cache_LRU[it].pid=pid;
		Cache_LRU[it].pag=pag;
		Cache_LRU[it].LRU=0;
		return it;
		}

}
return it; //WARN
}

int sustituirLRUproc(int pid, int pag)
{
	int it;

	int reemplazoLRU = maxLRUproc(pid);

	for(it=0;it<entradasCache;it++)
	{
		if (Cache_LRU[it].pid == pid && Cache_LRU[it].LRU== reemplazoLRU)
		{
			incrementarLRU(it);
			Cache_LRU[it].pid=pid;
			Cache_LRU[it].pag=pag;
			Cache_LRU[it].LRU=0;
			return it;
		}

	}
	return it;
}

void incrementarLRU(int posInicializar)
{
	int pos;
	for (pos=0; pos<entradasCache;pos++)
	{
		if (pos != posInicializar && Cache_LRU[pos].LRU != -1)
		{
			Cache_LRU[pos].LRU=Cache_LRU[pos].LRU+1;
		}
	}
}
int maxLRUproc(int pid)
{
	t_list* lt_pagsXpid= list_create();
	int primeraPosicion = 0;
	int z;
	int max=-1;
	for (z=0; z<entradasCache; z++ )
	{
		if (Cache_LRU[z].pid == pid)
		{
			list_add(lt_pagsXpid,(void *)Cache_LRU[z].LRU);
		}
	}
	list_sort(lt_pagsXpid,(void*) f_mayor);

	if ( list_size(lt_pagsXpid) != 0 ){
		max=(int)list_get(lt_pagsXpid,primeraPosicion);
	}
	return max;
}

char * almacenarBytes(int pid, int pag, int offset, int tam, char * buf)
{
	char *res;
	int frame_pos = posPaginaSolicitada(pid,pag);
	if (frame_pos <= 0){
		res=strdup("M030000000000");
		return res;
	} else {
		if(entradasCache != 0){ //CACHE HABILITADA
			int pEnCache = buscarPosCache(pid, pag);

			if (pEnCache == -1) //La pagina no se encuentra en CACHE
			{
				escribir_log("----------- Cache Miss - LA PAGINA SOLICITADA NO SE ENCUENTRA EN CACHE --------");
				escribir_log("----------- Actualizando Memoria Principal: Escritura ... -------------- ");
				sleep(retardo/1000);
				int pos = posFrameEnMemoria(frame_pos);
				memcpy(Memoria+pos+offset,buf,tam);
				res=strdup("M020000000000");
				return res;
			}
			else //La pagina se encuentra en CACHE.
			{
				escribir_log("---------  LA PAGINA SOLICITADA SE ENCUENTRA EN CACHE -----------");
				escribir_log("---------  Actualizando Memoria Cache: Escritura -----------");
				memcpy(Cache[pEnCache].dataFrame+offset, buf,tam);
				int pos = posFrameEnMemoria(frame_pos);
				escribir_log("-----------------  Actualizando Memoria Principal: Escritura ... ------------ ");
				sleep(retardo/1000);
				memcpy(Memoria+pos+offset,buf,tam);
				res=strdup("M020000000000");
				incrementarLRU(pEnCache);
				Cache_LRU[pEnCache].LRU = 0;
				//++++++++++++++++++++//
				// UpdateLRU(pid,pag);//
				//++++++++++++++++++++//
				return res;

			}
		}else{ // CACHE DESHABILITDA
			escribir_log(" ----------------  Actualizando Memoria Principal: Escritura ... -------------------");
			sleep(retardo/1000);
			int pos = posFrameEnMemoria(frame_pos);
			memcpy(Memoria+pos+offset,buf,tam);
			res=strdup("M020000000000");
			return res;
		}

	}
}

int posMaxLRU(void)
{
t_list* lt_pagsXpid= list_create();
int primeraPosicion = 0;
int z;
int max=-1;
for (z=0; z<entradasCache; z++ )
	 {
	 	list_add(lt_pagsXpid,(void *)Cache_LRU[z].LRU);
	 }
		list_sort(lt_pagsXpid,(void*) f_mayor);

	if ( list_size(lt_pagsXpid) != 0 ){
		max=(int)list_get(lt_pagsXpid,primeraPosicion);
	}
return max;
}

int liberar_paginas_pid(int pid){

int retorno = 0;
int count ;
int pag = 0;
int maximasPaginas = cantidadPaginas(pid);
printf("\n ULTIMA PAG ASIGNADA PAGINAS %d",maximasPaginas);

for(count=0;count < maximasPaginas;count ++){

 int posPag =posPaginaSolicitada(pid,pag);
 printf("\n POSPAG: %d \n",posPag);
 if (posPag == -1)
 {
	count --;
 }else{
	 retorno = liberar_una_pagina(pid,pag);
	 if(retorno == -1){
		 return retorno;
	 }
 }
	pag++;
}

eliminar_proceso(pid);
return retorno;

}

int liberar_una_pagina(int pid, int pag){

	int retorno;
	int hash_pos = hash(pid,pag);
	int re_hash_pos;
	int frame_pos;
	if (es_marco_correcto(pid, pag,hash_pos) == true ) {
		frame_pos = hash_pos;
	}
	else {
		re_hash_pos = buscar_marco_colision(pid,pag,hash_pos);
		frame_pos = re_hash_pos;
	}

/*	printf("\n Nro de Frame en Memo HASH:%d", hash_pos);
	printf("\n Nro de Frame en Memo REHASH:%d", re_hash_pos);
	printf("\n Nro de Frame en Memo POS:%d", frame_pos);
*/
	int eliminar_colisio =! es_marco_correcto(pid,pag,hash_pos);
	if(frame_pos == -1){
		// pagina inexistente
		retorno = -1;
	}else{
	if(entradasCache != 0){ // CACHE HABILITADA
		int pEnCache = buscarPosCache(pid, pag);
		if (pEnCache == -1) //La pagina no se encuentra en CACHE
		{
			tablaPaginas[frame_pos].estado = 0;
			tablaPaginas[frame_pos].pag = -1;
			tablaPaginas[frame_pos].pid = -1;

			if(eliminar_colisio == true){
				eliminar_colision(pid,pag,hash_pos);
			}

		}
		else //La pagina esta en CACHE
		{
			free(Cache[pEnCache].dataFrame);
			Cache[pEnCache].dataFrame = NULL;
			Cache[pEnCache].pag = -1;
			Cache[pEnCache].pid = -1;

			Cache_LRU[pEnCache].pag = -1;
			Cache_LRU[pEnCache].pid = -1;
			Cache_LRU[pEnCache].LRU = -1;

			tablaPaginas[frame_pos].estado = 0;
			tablaPaginas[frame_pos].pag = -1;
			tablaPaginas[frame_pos].pid = -1;

			if(eliminar_colisio == true){
				eliminar_colision(pid,pag,hash_pos);
			}
		}
	}else{ //CACHE DESHABILITADA

		tablaPaginas[frame_pos].estado = 0;
		tablaPaginas[frame_pos].pag = -1;
		tablaPaginas[frame_pos].pid = -1;

		if(eliminar_colisio == true){
			eliminar_colision(pid,pag,hash_pos);
		}
	}
	retorno = 0;
	}
	return retorno;
}
int posLibreMemoria(int pos)
{
 //Retorna 0=LIBRE 1=OCUPADO
if (tablaPaginas[pos].estado== -1 )
{
	return 0;
}
	return 1;

}
