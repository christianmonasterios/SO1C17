#ifndef KERNEL_PRUEBAS_SRC_SOCKET_CLIENT_H_
#define KERNEL_PRUEBAS_SRC_SOCKET_CLIENT_H_

int iniciar_socket_cliente(char*, int, int*);
int iniciar_socket_server(char*, int, int*);
int escuchar_conexiones(int, int*);
int aceptar_conexion(int, int*);
int enviar(int, char*, int*);
char *recibir(int, int*);
void cerrar_conexion(int);

#endif /* KERNEL_PRUEBAS_SRC_SOCKET_CLIENT_H_ */
