# Interconnect_for_MP_System

Este es el primer proyecto del curso CE-4302 Arquitectura de Computadores II del Programa de Licenciatura en Ingeniería en Computadores del Instituto Tecnológico de Costa Rica para el Semestre I 2025. Se hará un Modelado de un Interconnect para un sistema Multi-Procesador (MP).


1. Compilar el proyecto

Ir al folder Program con el Makefile.
```bash
cd Program/
```

Para compilar.
```bash
make
```

Para limpiar el proyecto.
```bash
make clean
```

2. Ejecutar la simulación

```bash
make run
```

3. Uso del Programa

El programa va a pedir que ingrese la cantidad de PEs. Ingresar un numero entre 8 y 32.

Luego saldrá el menu principal.

Se debe ir en orden para garantizar una correcta ejecución.

Ingresar 1 para generar los instruction files. Estos se pueden ver en la carpeta config/assemblers para verificar los workloads.

Volverá a aparecer el menú luego de ser creados.

Ingresar 2 para compilar los ensambladores. Estos se puedem ver en la carpeta config/binaries. Estos archivos contienen el bitstream utilizado por el programa.

Volverá a aparecer el menú luego de ser compilados.

Ingresar 3 para inicializar el sistema. Aqui sale otro prompt para escoger el esquema de arbitracion. Escoger el que guste.

Se procede a inicializar todos los objetos necesarios que contruyen el sistema y simularán el proceso del Interconnect.

Vuelve a aparecer el menú principal.

Ingresar 4 para Ejecutar la Simulación. Sale un prompt para escoger el modo de ejecucion, si es en stepping, se debe ir presionando [Enter] para avanzar. Si no, continuous, para un auto-run.

Empezará a ejecutarse y saldrán muchos prints. La latencia quedó alta por lo que durará un tiempo alto.

Si se abre el file Program/latency_log.txt se puede ir viendo como se van escribiendo los datos que se usarán en las estadisiticas y las gráficas.

Puede dar un error y no terminar la simulación, se encicla.

Al finalizar se pueden correr las simulaciones en un script aparte de Python ubicado en la carpeta Stats.
