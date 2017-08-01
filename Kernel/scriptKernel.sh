
cd src/


gcc Kernel.c configuracion.c consolaKernel.c consolaManager.c cpuManager.c fileSystem.c log.c manejo_conexiones.c manejo_errores.c memoria.c mensaje_consola.c mensaje.c metadata.c planificador.c semaforos_vglobales.c socket.c configuracion.h consolaKernel.h consolaManager.h cpuManager.h estructuras.h fileSystem.h log.h manejo_conexiones.h manejo_errores.h mensaje_consola.h mensaje.h metadata.h planificador.h semaforos_vglobales.h socket.h -o Kernel -lcommons -lpthread -lparser-ansisop

cd --

echo "Kernel compilado, ejecutar normalmente"
