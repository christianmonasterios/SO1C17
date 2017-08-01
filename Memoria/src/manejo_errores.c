#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include "estructuras.h"
#include "socket.h"
#include "log.h"

void error_sockets(int *controlador, char *proceso)
{
	switch (*controlador)
	{
		case 1:
			escribir_error_log("Memoria - Error creando socket");
			break;
		case 2:
			escribir_error_log("Memoria - Error conectando socket");
			break;
		case 3:
			escribir_error_log("Memoria - Error creando socket server");
			break;
		case 4:
			escribir_error_log("Memoria - Error bindeando socket server");
			break;
		case 5:
			escribir_error_log("Memoria - Socket server, error escuchando");
			break;
		case 6:
			escribir_error_log("Memoria - Error aceptando conexion");
			break;
		case 7:
			escribir_log_error_compuesto("Memoria - Error al enviar mensaje a: ", proceso);
			break;
		case 8:
			escribir_log_error_compuesto("Memoria - Error, socket desconectado", proceso);
			break;
		case 9:
			escribir_log_error_compuesto("Memoria - Error recibiendo mensaje de: ", proceso);
			break;
	}
}
