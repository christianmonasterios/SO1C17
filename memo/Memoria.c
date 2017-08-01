#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "estructuras.h"
#include "socket.h"
#include "log.h"
#include "mensaje.h"
#include "manejo_errores.h"
#include "hash.h"
#include <commons/collections/list.h>

//Variables Hilos x Pedido
int cliente[20];
pthread_t hiloEsperarMensaje[20];

// Variables Globales
int tamanioMarco;
int cantMarcos;
int socketServerMemoria;
int controlador = 0;
t_memoria *data_Memoria;
t_log *log_;
char * Memoria;
int STACK_SIZE;
int entradasCache;
int entradasProcesoCache;

//Estructuras administrativas

typedef struct{
	int estado;
	int pid;
	int pag;
} t_tablaPagina;

t_tablaPagina* tablaPaginas;

int cantMarcosTablaPag;

int tamTablaPaginas;

typedef struct{
	int pid;
	int pag;
	char * dataFrame;
} t_cache;

t_cache* Cache;


t_dictionary** colisiones;
int ultimo_frame_libre_asignado;

//Funciones
void leerArchivoConfig(char* rutaArchivoConfig);
void esperar_mensaje(void *i);
t_tablaPagina* crearEstrucTablaPag(void);
int cantidadDePaginas(int tamanioBytes);
void cargarCodigoEnMemoria(char * datos, char * Memoria ,t_list * frames_pedidos, int cantPaginasAgrabar);
int posFrameEnMemoria(int nroFrame);
int maxPaginaPid(int p_pid);
bool f_mayor(int a, int b);
void inicializarPrograma (int pid, int paginasRequeridas);
int posPaginaSolicitada(int pid,int paginaSolicitada);
//void almacenarBytes(int pid, int pag, int offset, int tamanio);
void mostrar_memoria (void);
void liberar_valor(char *val);
int cantMarcosLibre();
char * solicitarBytes(int pid, int pag, int offset, int tam);
char *almacenarBytes(int pid, int pag, int offset, int tam, char * buf);
t_cache* crearMemoCache(void);


int main(int argc, char *argv[])
{
	crear_archivo_log("/home/utnso/Escritorio/MEMORIA-DEBUGGING-LOG.txt");
	leerArchivoConfig(argv[1]);
	//Creacion de Socket Server

	printf(" \n");
	printf("CANT MARCOS: %d  \n",data_Memoria->MARCOS);
	printf("TAM MARCO: %d  \n",data_Memoria->MARCO_SIZE);

	tamanioMarco =data_Memoria->MARCO_SIZE;
	cantMarcos=data_Memoria->MARCOS;
	entradasCache = data_Memoria->ENTRADAS_CACHE;

	tamTablaPaginas = sizeof(t_tablaPagina)*cantMarcos;
	cantMarcosTablaPag = cantidadDePaginas(tamTablaPaginas);
    tablaPaginas = malloc(sizeof(t_tablaPagina)*cantMarcos);
    tablaPaginas = crearEstrucTablaPag();

    inicializar_array_colisiones();


    Memoria = malloc(cantMarcos * tamanioMarco);
    memset(Memoria,'\0',cantMarcos * tamanioMarco);

    Cache = malloc (sizeof(t_cache)*entradasCache); //(PID+PAG+*MARCO)*ENTRADASCACHE
    Cache = crearMemoCache();


	int d;
	for(d=0;d<entradasCache;d++){
		printf("\n %d | %d | %s \n",Cache[d].pid,Cache[d].pag,Cache[d].dataFrame);
	}


	printf("Tamaño de Tabla de paginas %d \n",tamTablaPaginas);
	printf("Tamaño Total de Memoria %d  \n",tamanioMarco * cantMarcos);

	//Cargo tabla de Paginas en Memoria

	int i_tp;
	int inicio=0;
	for(i_tp=0;i_tp<cantMarcosTablaPag;i_tp++){
	tablaPaginas[i_tp].estado = 1;
	tablaPaginas[i_tp].pag= inicio;
	inicio ++;
	}
	memcpy(Memoria,tablaPaginas,tamTablaPaginas);
	ultimo_frame_libre_asignado = cantMarcosTablaPag;

	int it;
	for(it=0;it<data_Memoria->MARCOS;it++){
		printf("-%d- %d | %d | %d \n",it,tablaPaginas[it].estado,tablaPaginas[it].pid,tablaPaginas[it].pag);
	}

	socketServerMemoria = iniciar_socket_server(data_Memoria->IP,data_Memoria->PUERTO,&controlador);

	escribir_log(string_from_format("Se inicia Servidor de Memoria en Socket: %d ",socketServerMemoria));



	int i = 0;

		while (1) {

			cliente[i] = escuchar_conexiones(socketServerMemoria,&controlador);

			escribir_log(string_from_format("Nueva Conexión entrante: %d ",cliente[i]));

	// Realizar HS y enviar Tamaño MARCO.
			char *mensaje_recibido = recibir(cliente[i], &controlador);
			char *header = get_header(mensaje_recibido);

			if(strcmp(header,"K")==0 || strcmp(header,"P")==0 )
				{
				//Cliente VALIDO
					char* HS_OK = armar_mensaje("M00",string_itoa(data_Memoria->MARCO_SIZE)); // M|00|0000000003|128|
					enviar(cliente[i],HS_OK, &controlador);
				}
				else
				{
					escribir_log(string_from_format("Se cierra la conexión con el Cliente luego del HandShake - Socket: %d ",cliente[i]));
					cerrar_conexion(cliente[i]);
					goto salir_handshake;
				}

	//Si es Cliente valido entra a interactuar con la MEMORIA. // Hilo

			pthread_create(&hiloEsperarMensaje[i], NULL, (void*) esperar_mensaje,(void *) cliente[i]);
			escribir_log(string_from_format("El Cliente %d tiene Hilo nro :%d ",cliente[i],i));

			// RECIBIR
			salir_handshake:
			i++;
		}

	//El select esta aca dentro --- ejecutar_server();
		free(tablaPaginas);
		free(Memoria);

	return EXIT_SUCCESS;

}

//A partir de este punto.. las funciones
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

void esperar_mensaje(void *i) {

	int cliente = (int) i;
	int chau = 0;
	while (chau != 1 )
	{
	char *mensRec = malloc(1024);
	memset(mensRec,'\0', 1024);

	mensRec = recibir(cliente, &controlador);

	if(controlador > 0)
	{
		cerrar_conexion(cliente);
		chau =1;
		goto chau;
	}

	char *header=get_header(mensRec);
	char *cod = get_codigo(mensRec);
	int codigo = atoi(cod);

	printf("\n header: %s \n",header);
	printf("\n codigo: %d \n",codigo);

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
										escribir_log(string_from_format("K06-Guardando codigo: %d",cliente));

										printf("\nMENSAJE RECIBIDO\n%s\n",mensRec);

										int procPid= atoi(string_substring(mensRec, 3, 4));

										char *paginasCodigo = string_substring(mensRec, 7, 4);
										int pagsCodigo = atoi(paginasCodigo);
										char *paginasSTACK = string_substring(mensRec, 11, 4);
										STACK_SIZE = atoi(paginasSTACK);

										printf("\n PID RECIBIDO %d \n",procPid);
										printf("\n TAMAÑO PAGINAS CODIGO %d \n",pagsCodigo);
										printf("\n TAÑANO PAGINAS STACK %d \n ",STACK_SIZE);

										int libres = cantMarcosLibre();

										if ( libres >= (pagsCodigo + STACK_SIZE))
										{
											inicializarPrograma (procPid,pagsCodigo);
											inicializarPrograma (procPid,STACK_SIZE);
											char* r_OK = armar_mensaje("M02","");
											enviar(cliente,r_OK, &controlador);
											escribir_log(string_from_format("K06- Se pudo guardar el CODIGO -: %d",cliente));
											//free(r_OK);

										} else
										{

											char* r_NOK = armar_mensaje("M03","");//M03
											enviar(cliente,r_NOK, &controlador);
											escribir_log(string_from_format("K06- NO se pudo guardar el CODIGO: %d",cliente));
											//free(r_NOK);
										}



										int it;
										for(it=0;it<cantMarcos;it++)
										{
											printf("-%d- %d | %d | %d \n",it,tablaPaginas[it].estado,tablaPaginas[it].pid,tablaPaginas[it].pag);
										}


										free(paginasCodigo);
										free(paginasSTACK);

									}
								break;
								case 20: //K|18|CANTIDADBYTES|CODIGO|PID
											{
											char *escribir =string_from_format("K06 - Guardando codigo: %d ",cliente);
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

											printf("\n ----------------------------- \n");
											printf("%s \n", mensRec);
											printf("%d \n",procPid);
											printf("%d \n",pagsCod);
											printf("%s", codigo);
											printf("\n ----------------------------- \n");
										//Meter funciones de guardar codigo

											int i;
											for (i=0;i<pagsCod; i++)
											{
											int frame_pos = posPaginaSolicitada(procPid,i);
											printf("\n POS: %d \n",frame_pos);
											int pos = posFrameEnMemoria(frame_pos);
											memcpy(Memoria+pos,codigo+(i*tamanioMarco),tamanioMarco);
											}
											free(codigo);

											mostrar_memoria ();


										//free(mensRec);
										}
										break;

				case 99: //INICIALIZAR PROGRAMA
													{
														mostrar_memoria();

													}
													break;
				}
//				free(cod);
//				free(mensRec);
//				free(header);
	} else
		switch (codigo)
						{
										case 01: //Solicitar Bytes de Memoria
											{
												char *ppid = string_substring(mensRec, 3, 4);
												int pid = atoi(ppid);
												char *ppag = string_substring(mensRec, 7, 4);
												int pag = atoi(ppag);
												char *poffset = string_substring(mensRec, 11, 4);
												int offset= atoi(poffset);
												char *ptam = string_substring(mensRec, 15, 4);
												int tam = atoi(ptam);

												printf("\n Solicitar Bytes \nPID:%d \nPAG:%d \nOFFSET:%d \nTAM:%d\n",pid, pag, offset,tam);
												char * Buffer = solicitarBytes(pid, pag, offset, tam);
												printf("\n%s\n", Buffer);
												char *buffer_aux=string_substring(Buffer,0,tam);
												//armarmensaje y sale-
												enviar(cliente,buffer_aux, &controlador);
												free(buffer_aux);
												free(ppid);
												free(ppag);
												free(ptam);
												free(poffset);
												free(Buffer);
											}

											break;
										case 8: //K|18|PIDD|CANTIDADBYTES|CODIGO
											{
												printf("\n CASE 20 DE CPU\n");

												char *ppid = string_substring(mensRec, 3, 4);
												int pid = atoi(ppid);
												char *ppag = string_substring(mensRec, 7, 4);
												int pag = atoi(ppag);
												char *poffset = string_substring(mensRec, 11, 4);
												int offset= atoi(poffset);
												char *ptam = string_substring(mensRec, 15, 4);
												int tam = atoi(ptam);
												char *pbuffer = string_substring(mensRec, 19, tam);

												printf("\n Almacenar Bytes \nPID:%d \nPAG:%d \nOFFSET:%d \nTAM:%d\n",pid, pag, offset,tam);
												printf("\n %s",pbuffer);

												char *r_OK = almacenarBytes(pid, pag, offset, tam, pbuffer);
												//char* r_OK = armar_mensaje("M02","");
												enviar(cliente,r_OK, &controlador);


												free(ppid);
												free(ppag);
												free(ptam);
												free(pbuffer);
												free(poffset);
											}
										break;


						}

				chau:
				printf("\n PROCESADO \n");

				free(cod);
				free(header);
				free(mensRec);

	}
	}


t_tablaPagina* crearEstrucTablaPag(void){

	t_tablaPagina* estructuraTablaPaginas=malloc(sizeof(t_tablaPagina)*cantMarcos);
	int i;
	for(i=0;i<data_Memoria->MARCOS;i++){
		estructuraTablaPaginas[i].estado=0;
		estructuraTablaPaginas[i].pag=-1;
		estructuraTablaPaginas[i].pid=-1;
	}
	return estructuraTablaPaginas;
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

void cargarCodigoEnMemoria(char * datos, char * Memoria ,t_list * frames_pedidos, int cantPaginasAgrabar) {
	int c = 0;
	while(c < cantPaginasAgrabar){
		int frame_pos = (int)list_get(frames_pedidos, c);
		int pos = posFrameEnMemoria(frame_pos);
		memcpy(Memoria+pos,datos+(c*tamanioMarco),tamanioMarco);
		printf("\nSe guardo en el frame: %d \n", frame_pos);
		c++;
	}
}

/*void almacenarBytes(int pid, int pag, int offset, int tamanio)
{
	int c = 0;

	while(c < ){
		int frame_pos = list_get(frames_pedidos, c);
		int pos = posFrameEnMemoria(frame_pos);
		memcpy(Memoria+pos,datdatosos+(c*tamanioMarco),tamanioMarco);
		printf("\nSe guardo en el frame: %d \n", frame_pos);
		c++;
	}

}
*/


int posFrameEnMemoria(int nroFrame){
	//Devuelve la posicion de inicio del Frame
	int pos= (nroFrame* tamanioMarco)-1;
	return pos;
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
	 		list_add(lt_pagsXpid,tablaPaginas[z].pag);
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


void inicializarPrograma (int pid,int cantPaginasAgrabar)
	{
		//Calcular posición frames libres más próximos para una cantidad solicitada.
		int maxPagPid = maxPaginaPid(pid); //Funcion que me devuelva la máxima pagina de un PID.

		int pos_hash;
		int pos_ReHash;
		int ultimoframe;
		int i;

		for (i = 0; i< cantPaginasAgrabar;i++)
		{
		int pag = maxPagPid +1;
		printf("\n ANTES DE LA HASHH: \n");
		pos_hash= hash(pid, pag );
		printf("\n pos_has: %d \n",pos_hash);

		if (marco_ocupado(pos_hash) == false ){
			ultimoframe = pos_hash;
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
			printf("\n pos_RE_has: %d \n",pos_ReHash);
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

void liberar_valor(char *val)
{
	free(val);
}

void mostrar_memoria (void)
{
int a;
printf("\n ------------------------------------------- \n");
for (a = cantMarcosTablaPag; a<cantMarcos; a ++ )
{
int pos = posFrameEnMemoria(a);
char * dataFrame = malloc (tamanioMarco); memset(dataFrame,'\0',tamanioMarco);
//dataFrame[tamanioMarco]='\0';
memcpy(dataFrame,Memoria+pos,tamanioMarco);
//
//printf("|%s| - %d \n",dataFrame,a);
char * imp = string_from_format("|%s| - %d \n",dataFrame,a);
escribir_log(imp);
free(imp);

free(dataFrame);
}
printf("\n ------------------------------------------- \n");
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
	printf("\n CANTIDAD MARCOS LIBRES:%d \n", a);
	return a;
}

char * solicitarBytes(int pid, int pag, int offset, int tam)
{
	int frame_pos = posPaginaSolicitada(pid,pag);
	printf("\n frame_pos: %d \n", frame_pos);

	if (frame_pos == 0){
		char * rta = strdup("-1");
		return rta;
	}

	int pos = posFrameEnMemoria(frame_pos);
	char * dataFrame = malloc (tam);
	memset(dataFrame,'\0',tam);
	//dataFrame[tamanioMarco]='\0';
	memcpy(dataFrame,Memoria+pos+offset,tam);
	return dataFrame;
}

char* almacenarBytes(int pid, int pag, int offset, int tam, char * buf)
{
	char *res;
	int frame_pos = posPaginaSolicitada(pid,pag);
	if(frame_pos <= 0){
		res = strdup("M030000000000");
	}else{

	int pos = posFrameEnMemoria(frame_pos);
	escribir_log_con_numero("Posicion en MRAM",frame_pos);
	escribir_log_con_numero("Pos MRAM",pos);
	//dataFrame[tamanioMarco]='\0';
	memcpy(Memoria+pos+offset,buf,tam);
	res=strdup("M020000000000");
	}
	return res;
}

t_cache* crearMemoCache(void){

	t_cache* MemoCache=malloc(sizeof(t_cache)*entradasCache);
	int i;
	for(i=0;i<entradasCache;i++){
		MemoCache[i].pag=-1;
		MemoCache[i].pid=-1;
	}
	return MemoCache;
}

