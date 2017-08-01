#!/bin/bash

cd --
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library

make

sudo make install

cd --

git clone https://github.com/sisoputnfrba/ansisop-parser

cd ansisop-parser/parser

make all

sudo make install

cd --

echo "Commons y Parser listos"

cd /home/utnso/tp-2017-1c-TodaviaSirve/evaluacion-final-esther/FS-ejemplo

tar -xzvf FS.tgz
tar -xzvf SADICA_FS_V2.tar.gz

cd --

mv /home/utnso/tp-2017-1c-TodaviaSirve/evaluacion-final-esther/FS-ejemplo/SADICA_FS /home/utnso/FS_SADICA

echo "FileSystem descomprimido en /home/utnso/FS_SADICA"

echo "Entre a la carpeta del proyecto que desea copilar, modifique los permisos del script y ejecutelo haciendo ./*nombre script*"
echo "Si no hubo errores, el ejecutable se escontrara en la carpeta src, ejecutelo con normalidad"
echo "¡ Exitos !"



