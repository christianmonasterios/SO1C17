#ifndef CPU_H_
#define CPU_H_

int iniciar_socket_cliente(char*, int, int*);
int enviar(int, void *, int*,int);
void recibir(int, int*, void *,int);
void cerrar_conexion(int);

#endif /* CPU_H_ */
