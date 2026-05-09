// base_bounds.c
#include <stdio.h>

typedef struct {
    int base;
    int bounds;
} Registro;

/* Traduce VA -> PA; imprime excepcion si viola bounds */
int traducir(Registro r, int va) {
    if (va < 0 || va >= r.bounds) {
        printf(" [EXCEPCION] VA=%d viola bounds=%d\n", va, r.bounds);
        return -1;
    }

    return r.base + va;
}

int main() {
    Registro procA = {32, 64};    /* base=32, bounds=64 */
    Registro procB = {128, 80};   /* base=128, bounds=80 */

    int vas[] = {0, 10, 63, 64, 100};
    int n = sizeof(vas) / sizeof(vas[0]);

    printf("--- Proceso A (base=%d, bounds=%d) ---\n",
           procA.base, procA.bounds);

    for (int i = 0; i < n; i++) {
        int pa = traducir(procA, vas[i]);

        if (pa != -1) {
            printf(" VA=%3d -> PA=%3d\n", vas[i], pa);
        }
    }

    printf("--- Proceso B (base=%d, bounds=%d) ---\n",
           procB.base, procB.bounds);

    for (int i = 0; i < n; i++) {
        int pa = traducir(procB, vas[i]);

        if (pa != -1) {
            printf(" VA=%3d -> PA=%3d\n", vas[i], pa);
        }
    }

    return 0;
}