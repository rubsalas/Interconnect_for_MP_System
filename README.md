# Interconnect_for_MP_System
Este es el primer proyecto del curso CE-4302 Arquitectura de Computadores II del Programa de Licenciatura en Ingeniería en Computadores del Instituto Tecnológico de Costa Rica para el Semestre I 2025. Se hará un Modelado de un Interconnect para un sistema Multi-Procesador (MP).

* Compilador

Las instrucciones soportadas por los workloads de pruebas poseen el siguiente formato

- WRITE_MEM <SRC>, <ADDR>, <NUM_OF_CACHE_LINES>, <START_CACHE_LINE>, <QoS>
- READ_MEM <SRC>, <ADDR>, <SIZE>, <QoS>
- BROADCAST_INVALIDATE <SRC>, <CACHE_LINE>, <QoS>

Donde:
 <SRC> es un numero de 5 bits el cual representa el PE que envia la instruccion, 32 PEs posibles
 <ADDR> es un numero de 16 bits representando la direccion de memoria compartida
 <NUM_OF_CACHE_LINES> es un numero de 8 bits representando la linea de cache
 <START_CACHE_LINE> es un numero de 8 bits que representa la linea inicial de cache
 <QoS> es un numero de 4 bits representando la prioridad 
 <SIZE> es un numero de 8 bits indicando el tamano del bloque de cache por leer.

En este archivo puede ser visualizado el acomodo de los bits de las instrucciones una vez compiladas:
 
https://docs.google.com/spreadsheets/d/1nA-x_ndPWorXsLAwrE7hO1Qq5rFNstHbkzMkOFf6aBE/edit?usp=sharing

Para el compilador se debe realizar de la siguiente manera:

g++ -std=c++20 Compiler.cpp main_compiler.cpp -o compiler
./compiler -i test_input.asm -o output.txt
