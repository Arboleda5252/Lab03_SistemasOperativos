# Laboratorio 03 (Gestión de Memoria) - Sistemas Operativos 
Elaborado por: 
- Carlos Andres Zuluaga Amaya
  andres.zuluaga6@udea.edu.co
- Duván Antonio Arboleda Botero
  duvan.arboleda1@udea.edu.co

Link de video: 

Link de informe en pdf: 

## Introducción
En este laboratorio se estudia cómo los sistemas operativos administran la memoria. Se usan programas en C y herramientas de Linux como /proc, pmap y Valgrind.
Se analizan conceptos como espacio de direcciones, heap, stack, segmentación, paginación, fragmentación y TLB.

## Objetivo
- Identificar las regiones de memoria de un proceso: código, heap y stack.
- Detectar errores de memoria con Valgrind.
- Comprender los mecanismos de Base & Bounds, segmentación y paginación.
- Analizar cómo la fragmentación y el TLB afectan el uso y rendimiento de la memoria.

## Herramientas
Para este laboratorio es necesario tener instalado un compilador de C, principalmente gcc y también es necesario instalar Valgrind, para detectar errores de memoria.

### Compilación
```bash
gcc-Wall-o nombre nombre.c
```

### Valgrind
```bash
valgrind--leak-check=full ./nombre
```

La herramienta Valgrind se instala con `sudo apt install valgrind`

## Desarrollo

