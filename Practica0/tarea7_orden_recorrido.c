#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define N 80

int main(int argc, char *argv[]) {
    FILE *f;
    uint8_t *VectIncrementando = malloc(N * sizeof(uint8_t));
    uint8_t *VectDecrementando = malloc(N * sizeof(uint8_t));
    bool procesadoInc[N];
    bool procesadoDec[N];
    int i;

    /* inicializaciones */
    memset(procesadoInc, 0, sizeof(procesadoInc));
    memset(procesadoDec, 0, sizeof(procesadoDec));

    if (argc < 2) {
        printf("Uso: %s fichero80bytes.bin\n", argv[0]);
        printf("El programa necesita un archivo binario con al menos 80 bytes.\n");
        return 1;
    }

    f = fopen(argv[1], "rb");
    fread(VectIncrementando, 1, N, f);
    fclose(f);

    /* copia al otro vector */
    memcpy(VectDecrementando, VectIncrementando, N);

    /* --- Recorrido incrementando (i = 0..79) sobre VectIncrementando --- */
    for (i = 0; i < N; ++i) {
        if (i >= 3) {
            int s = VectIncrementando[i-1]
                  + VectIncrementando[i-2]
                  - VectIncrementando[i-3];
            if ((int)VectIncrementando[i] > s) {
                VectIncrementando[i] = (uint8_t)s;
            } else {
                VectIncrementando[i] = (uint8_t)(VectIncrementando[i] / 2);
            }
            procesadoInc[i] = true;
        } else {
            /* no se puede procesar (i = 0,1,2) */
            procesadoInc[i] = false;
        }
    }

    

    /* --- Recorrido decrementando (i = 79..0) sobre VectDecrementando --- */
    for (i = N - 1; i >= 0; --i) {
        if (i >= 3) {
            int s = (int)VectDecrementando[i-1]
                  + (int)VectDecrementando[i-2]
                  - (int)VectDecrementando[i-3];
            if ((int)VectDecrementando[i] > s) {
                VectDecrementando[i] = (uint8_t)s;
            } else {
                VectDecrementando[i] = (uint8_t)(VectDecrementando[i] / 2);
            }
            procesadoDec[i] = true;
        } else {
            /* no se puede procesar (i = 0,1,2) */
            procesadoDec[i] = false;
        }
    }

    /* --- Responder la plantilla: listar no procesados y comparar --- */

    printf("\n--- Qué elementos NO procesados en VectIncrementando (índice : valor original) ---\n");
    for (i = 0; i < N; ++i) {
        if (!procesadoInc[i]) {
            /* los valores no procesados siguen siendo los originales leídos */
            /* Para obtener el valor original usaremos VectDecrementando (era copia al principio)
               pero ojo: VectDecrementando pudo haber sido modificado después.
               Para mayor claridad guardamos los originales antes de procesar:
               (sin hacer comprobaciones complejas: se podría haber guardado otra copia).
               Como no se guardó explícitamente al principio, asumimos que los índices no procesados
               (0..2) no fueron tocados por ninguno de los dos bucles, por lo que su valor actual es el original. */
            printf("i=%2d : %3u\n", i, (unsigned)VectIncrementando[i]);
        }
    }

    printf("\n--- Qué elementos NO procesados en VectDecrementando (índice : valor original) ---\n");
    for (i = 0; i < N; ++i) {
        if (!procesadoDec[i]) {
            printf("i=%2d : %3u\n", i, (unsigned)VectDecrementando[i]);
        }
    }

    /* comparar cuantos elementos coinciden / distintos */
    int iguales = 0, distintos = 0;
    for (i = 0; i < N; ++i) {
        if (VectIncrementando[i] == VectDecrementando[i]) iguales++;
        else distintos++;
    }

    printf("\n--- Comparación elemento a elemento ---\n");
    printf("Elementos con mismo valor en ambos vectores:   %d\n", iguales);
    printf("Elementos con distinto valor en ambos vectores: %d\n", distintos);

    /* Para ayudar a inspeccionar, opcionalmente listamos diferencias (índice : inc -> dec) */
    printf("\nÍndices con diferencia (i : VectIncrementando -> VectDecrementando):\n");
    for (i = 0; i < N; ++i) {
        if (VectIncrementando[i] != VectDecrementando[i]) {
            printf("i=%2d : %3u -> %3u\n", i,
                   (unsigned)VectIncrementando[i],
                   (unsigned)VectDecrementando[i]);
        }
    }

    /* Conclusión simple basada en los resultados */
    printf("\n--- Conclusión ---\n");
    printf("Índices no procesados por ambos: normalmente i = 0,1,2 (porque faltan i-1,i-2,i-3).\n");
    printf("Las diferencias entre los vectores ocurren porque el recorrido ascendente modifica\n"
           "valores de índices bajos antes de usarlos para índices mayores, mientras que el\n"
           "recorrido descendente siempre usa los valores 'inferiores' originales para calcular\n"
           "los superiores (debido al orden de procesamiento). Por tanto, el orden de iteración\n"
           "impacta en el resultado final cuando la regla depende de elementos previamente modificados.\n");

    /* Limpieza */
    free(VectIncrementando);
    free(VectDecrementando);

    return 0;
}