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
valgrind --leak-check=full ./nombre
```

La herramienta Valgrind se instala con `sudo apt install valgrind`

![texto](IMG/1.png)

## Desarrollo

### 1. Espacio de Direcciones

En este paso vamos a crear el archivo `mem_map.c` donde imprime direcciones virtuales de distintas zonas de memoria: código, variable global, stack y heap.

```c
// mem_map.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_var = 42; /* segmento de datos (.data) */

int main() {
    int local_var = 10;                    /* stack */
    int *heap_var = malloc(sizeof(int)*100); /* heap */

    if (heap_var == NULL) {
        perror("malloc");
        return 1;
    }

    *heap_var = 99;

    printf("PID del proceso     : %d\n", getpid());
    printf("Dir. codigo (main)  : %p\n", (void*)main);
    printf("Dir. global_var     : %p\n", (void*)&global_var);
    printf("Dir. local_var      : %p\n", (void*)&local_var);
    printf("Dir. heap_var       : %p\n", (void*)heap_var);

    printf("\nPresione ENTER para continuar...\n");
    getchar();

    free(heap_var);
    return 0;
}
```

Vamos a compilar y ejecutar el programa

![texto](IMG/1-1.png)

Al ejecutar ambos procesos, vemos que cada proceso tiene su propio espacio de direccion privado, administrado por el sistema operativo.

![texto](IMG/1-2.png)

#### 1. Identifique en la salida de /proc/maps las regiones `text`, `heap` y `stack`. ¿Que permisos (r/w/x/p) tiene cada una? ¿Por qué difieren?

En la salida de /proc/[pid]/maps:
- text: corresponde al ejecutable mem_map, sus permisos aparece como r-x-- (lectura y ejecución)
- heap: memoria dinámica reservada con malloc, sus permisos aparece comos rw--- (Consultar y guardar datos)
- stack: se guardan variables locales, direcciones de retorno y datos temporales, sus funciones aparece como rw--- (Escribir y leer constantemente durante la ejecución)

Estos permisos están diseñados para garantizar tanto la funcionalidad del programa como la seguridad del sistema operativo, y son los siguientes: 
- r (read): leer instrucciones
- w (write): escritura
- x (execute): ejecución
- p (private): Indica que la memoria es privada para el proceso.

#### 2. Compare las direcciones impresas con los rangos de /proc/maps. ¿A qué región pertenece cada variable?

| Variable | Región en /proc/maps | Rango de direcciones |
|---|---|---|
| main | r-xp (mem_map) | 62e01eb8d000 - 62e01eb8e000 |
| global_var | rw-p (mem_map) | 62e01eb90000 - 62e01eb91000 |
| local_var | [stack] | 62e04a9e9000 - 62e04aa0a000 |
| heap_var | [heap] | 7ffcac602000 - 7ffcac623000 |

#### 3. ¿Qué otras regiones aparecen en el mapa (`libc`, `[vdso]`, `[vsyscall]`)? ¿Que función cumple cada una?

- libc: biblioteca estándar de C, usada por funciones como printf, malloc y free.
- [vdso]: ejecutar algunas llamadas al sistema de forma más rápida.
- [vsyscall]: predecesor de vdso, relacionado con llamadas rápidas al sistema, mantenido por compatibilidad.
- [vvar]: zona con datos del kernel usados por el vdso.

#### 4. ¿Son las direcciones virtuales iguales a las fisicas? Explique apoyandose en el concepto de address space del OSTEP.

Falso. Las direcciones que imprime el programa son direcciones virtuales, no físicas. Cada proceso cree tener su propio espacio de direcciones privado y continuo, pero el hardware traduce esas direcciones virtuales a direcciones físicas reales. 

#### Comparar espacios de dos procesos simultáneos

Al ejecutar ambos procesos, vemos que cada proceso tiene su propio espacio de direccion privado, administrado por el sistema operativo.

| Terminal 1 | Terminal 2 |
|---|---|
| ![texto](IMG/1-2.png) | ![texto](IMG/1-2.png) |

**¿Son las mismas direcciones virtuales en ambos procesos? ¿Qué conclusión se saca sobre el aislamiento del espacio de direcciones?**

Se observó que las direcciones son diferentes, es decir, cada proceso tiene un espacio de direcciones virtual independiente. El sistema operativo le da a cada proceso su propia memoria privada. Esto permite que un proceso no pueda acceder directamente a la memoria de otro proceso, aunque ambos ejecuten el mismo programa.

**¿Podría el Proceso A leer o modificar la variable global del Proceso B mediante su dirección virtual? Justifique.**

No. El Proceso A no puede leer ni modificar la variable global del Proceso B usando su dirección virtual. Esto demuestra el aislamiento entre procesos: cada proceso tiene su propia tabla de páginas y sus propias traducciones de direcciones virtuales a direcciones físicas. 

### 2. API de Memoria



