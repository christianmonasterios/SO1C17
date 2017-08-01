#ifndef SOCKET_CLIENT_H_
#define SOCKET_CLIENT_H_

//Esta funcion genera un socket de tipo cliente (no puede aceptar conexiones, solo se conecta)
int iniciar_socket_cliente(char *ip, int puerto/*char *puerto*/);

//Esta funcion envia mensajes a traves del socket conectado
//Devuelve un entero significativo de que si el mensaje fue enviado o no
int enviar(int socket_emisor, char *mensaje_a_enviar, int tamanio);

//Esta funcion recibe mensajes a traves de un socket
char *recibir(int socket_receptor,int tamanio);
//Esta funcion elimina el socket
void cerrar_conexion(int socket_);

#endif /* SOCKET_CLIENT_H_ */
