#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include "estructuras.h"
#include "socket.h"
#include "mensaje.h"

void avisar_consola_proceso_murio(t_program *prog)
{
	int controlador;
	char *pid_char = string_itoa(prog->PID);
	char *mensaje_enviar = armar_mensaje("K10", pid_char);

	enviar(prog->socket_consola, mensaje_enviar, &controlador);

	free(pid_char);
	free(mensaje_enviar);
}
