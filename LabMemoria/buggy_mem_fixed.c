// buggy_mem_fixed.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int *p = malloc(5 * sizeof(int));

    if (p == NULL) {
        perror("malloc p");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        p[i] = i;
    }

    char *q = malloc(100);

    if (q == NULL) {
        perror("malloc q");
        free(p);
        return 1;
    }

    strcpy(q, "hola mundo");
    printf("%s\n", q);

    printf("p[0] = %d\n", p[0]);

    free(p);
    free(q);

    return 0;
}