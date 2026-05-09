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

#### Programa base

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

#### Visualizar los mapas de memoria de un proceso
Al ejecutar ambos procesos, vemos que cada proceso tiene su propio espacio de direccion privado, administrado por el sistema operativo.

![texto](IMG/1-2.png)

#### Exploración de /proc/[pid]/maps

**1. Identifique en la salida de /proc/maps las regiones `text`, `heap` y `stack`. ¿Que permisos (r/w/x/p) tiene cada una? ¿Por qué difieren?**

En la salida de /proc/[pid]/maps:
- text: corresponde al ejecutable mem_map, sus permisos aparece como r-x-- (lectura y ejecución)
- heap: memoria dinámica reservada con malloc, sus permisos aparece comos rw--- (Consultar y guardar datos)
- stack: se guardan variables locales, direcciones de retorno y datos temporales, sus funciones aparece como rw--- (Escribir y leer constantemente durante la ejecución)

Estos permisos están diseñados para garantizar tanto la funcionalidad del programa como la seguridad del sistema operativo, y son los siguientes: 
- r (read): leer instrucciones
- w (write): escritura
- x (execute): ejecución
- p (private): Indica que la memoria es privada para el proceso.

**2. Compare las direcciones impresas con los rangos de /proc/maps. ¿A qué región pertenece cada variable?**

| Variable | Región en /proc/maps | Rango de direcciones |
|---|---|---|
| main | r-xp (mem_map) | 62e01eb8d000 - 62e01eb8e000 |
| global_var | rw-p (mem_map) | 62e01eb90000 - 62e01eb91000 |
| local_var | [stack] | 62e04a9e9000 - 62e04aa0a000 |
| heap_var | [heap] | 7ffcac602000 - 7ffcac623000 |

**3. ¿Qué otras regiones aparecen en el mapa (`libc`, `[vdso]`, `[vsyscall]`)? ¿Que función cumple cada una?**

- libc: biblioteca estándar de C, usada por funciones como printf, malloc y free.
- [vdso]: ejecutar algunas llamadas al sistema de forma más rápida.
- [vsyscall]: predecesor de vdso, relacionado con llamadas rápidas al sistema, mantenido por compatibilidad.
- [vvar]: zona con datos del kernel usados por el vdso.

**4. ¿Son las direcciones virtuales iguales a las fisicas? Explique apoyandose en el concepto de address space del OSTEP.**

Falso. Las direcciones que imprime el programa son direcciones virtuales, no físicas. Cada proceso cree tener su propio espacio de direcciones privado y continuo, pero el hardware traduce esas direcciones virtuales a direcciones físicas reales. 

#### Comparar espacios de dos procesos simultáneos

Al ejecutar ambos procesos, vemos que cada proceso tiene su propio espacio de direccion privado, administrado por el sistema operativo.

| Terminal 1 | Terminal 2 |
|---|---|
| ![texto](IMG/1-4-1.png) | ![texto](IMG/1-4-2.png) |

**1. ¿Son las mismas direcciones virtuales en ambos procesos? ¿Qué conclusión se saca sobre el aislamiento del espacio de direcciones?**

Se observó que las direcciones son diferentes, es decir, cada proceso tiene un espacio de direcciones virtual independiente. El sistema operativo le da a cada proceso su propia memoria privada. Esto permite que un proceso no pueda acceder directamente a la memoria de otro proceso, aunque ambos ejecuten el mismo programa.

**2. ¿Podría el Proceso A leer o modificar la variable global del Proceso B mediante su dirección virtual? Justifique.**

No. El Proceso A no puede leer ni modificar la variable global del Proceso B usando su dirección virtual. Esto demuestra el aislamiento entre procesos: cada proceso tiene su propia tabla de páginas y sus propias traducciones de direcciones virtuales a direcciones físicas. 

### 2. API de Memoria

#### Programa base
En este paso vamos a crear el archivo `heap_demo.c` que reserva memoria dinámica en el heap usando malloc para almacenar, se usa realloc para ampliar el bloque de memoria y se libera la memoria con free.

```c
// heap_demo.c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 10;
    int *arr = (int *) malloc(n * sizeof(int));

    if (arr == NULL) {
        perror("malloc");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        arr[i] = i * i;
    }

    printf("Arreglo original: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    /* Redimensionar a 20 enteros */
    arr = (int *) realloc(arr, 20 * sizeof(int));

    if (arr == NULL) {
        perror("realloc");
        return 1;
    }

    for (int i = n; i < 20; i++) {
        arr[i] = i * i;
    }

    printf("Arreglo ampliado: ");
    for (int i = 0; i < 20; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    free(arr);

    return 0;
}
```

Vamos a compilar y ejecutar el programa

![texto](IMG/2-1-1.png)

#### Uso correcto de malloc y free

**1. Muestre la salida completa de Valgrind. ¿Reporta errores o fugas de memoria? ¿Que significa el mensaje "´All heap blocks were freed´"?** 

![texto](IMG/2-1-0.png)

No. Valgrind no reporta errores ni fugas de memoria. El resumen indica `ERROR SUMMARY: 0 errors from 0 contexts`

`All heap blocks were freed -- no leaks are possible`: toda la memoria reservada dinámicamente durante la ejecución fue liberada correctamente antes de terminar el programa.

**2. ¿Porqué ese usa sizeof(int) en lugar del valor literal 4? ¿Qué ventaja ofrece en portabilidad entre arquitecturas?**

Se usa `sizeof(int)` porque el tamaño de un int puede depender de la arquitectura y el compilador.

**3. ¿Qué devuelve malloc cuando no hay memoria disponible? ¿Porqué es critico verificar ese valor antes de usarlo?**

Cuando malloc no puede reservar memoria, devuelve NULL. Esta verificación es crítica porque, si malloc falla y el programa intenta usar el puntero estaría accediendo a una dirección inválida. 

#### Código con bugs de memoria

Se creó el archivo buggy_mem.c, el cual contiene tres errores de manejo de memoria.

```c
// buggy_mem.c -- NO ejecutar sin Valgrind
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    /* ERROR 1: buffer overflow */
    int *p = malloc(5 * sizeof(int));

    for (int i = 0; i <= 5; i++) { /* <= en vez de < */
        p[i] = i;
    }

    /* ERROR 2: memory leak, nunca se llama free(q) */
    char *q = malloc(100);
    strcpy(q, "hola mundo");
    printf("%s\n", q);

    /* ERROR 3: use-after-free */
    free(p);
    printf("p[0] = %d\n", p[0]); /* acceso ilegal */

    return 0;
}
```

La ejecución con Valgrind permite detectar estos errores en tiempo de ejecución

![texto](IMG/2-3.png)

#### Identificar y corregir errores de memoria

**1. Transcriba los mensajes que arroja Valgrind. ¿Cual mensaje corresponde a cada uno de los tres errores clásicos?**

- Error 1: Buffer Overflow,
  Mensaje de Valgrind: ==2523== Invalid write of size 4 
- Error 2: Memory Leak
  Mensaje de Valgrind: ==2523== LEAK SUMMARY 
- Error 3: Acceso Ilegal 
  Mensaje de Valgrind: ==2523== Invalid read of size 4

**2. Corrija el programa (buggy-mem-fixed.c) y verifique con Valgrind que no queda ningun error y ni fuga**

Código corregido:

```
// buggy_mem_fixed.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    /* Correccion 1: reservar 5 enteros y usar solo indices 0 a 4 */
    int *p = malloc(5 * sizeof(int));

    if (p == NULL) {
        perror("malloc p");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        p[i] = i;
    }

    /* Correccion 2: reservar memoria para q y liberarla al final */
    char *q = malloc(100);

    if (q == NULL) {
        perror("malloc q");
        free(p);
        return 1;
    }

    strcpy(q, "hola mundo");
    printf("%s\n", q);

    /* Usar p antes de liberarlo */
    printf("p[0] = %d\n", p[0]);

    /* Liberar toda la memoria reservada */
    free(p);
    free(q);

    return 0;
}
```

Después de compilar y ejecutar buggy_mem_fixed.c con Valgrind, se verificó que no quedaran errores ni fugas de memoria.

![texto](IMG/2-4-2.png)

**3. ¿Qué consecuencias puede tener un use-after-free en un programa real en términos de seguridad y estabilidad del sistema?**

Un use-after-free puede causar problemas graves en un programa real. En términos de estabilidad, puede producir resultados incorrectos, bloqueos inesperados porque el programa está usando memoria que ya no le pertenece. En términos de seguridad, es peligroso. Si esa memoria liberada es reutilizada para otros datos, un atacante podría manipular el contenido y lograr corrupción de memoria.

### 3. Traducción de direcciones — Base & Bounds



