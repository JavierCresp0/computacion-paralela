/* Simple y directo - C (no C++).
   - MatTotal: unsigned char** con malloc por fila (forma "literal" de puntero doble).
   - Lee fichero fila a fila.
   - Copia 4 cuadrantes 256x256 en CuadSI,CuadSD,CuadII,CuadID (malloc por fila).
   - Libera MatTotal y calcula la desviación estándar de filas dadas reconstruyéndolas. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 512
#define Q 256

int main(void)
{
    int i, j, c;
    FILE *f = fopen("fichero2.bin", "rb");

    /* MatTotal: malloc por fila (puntero doble "literal") */
    unsigned char **MatTotal = malloc(N * sizeof(unsigned char*));
    for (i = 0; i < N; ++i)
        MatTotal[i] = malloc(N * sizeof(unsigned char));

    /* leer fila a fila */
    for (i = 0; i < N; ++i)
        fread(MatTotal[i], sizeof(unsigned char), N, f);
    fclose(f);

    /* reservar 4 cuadrantes (cada uno Q filas, cada fila Q bytes) */
    unsigned char **SI = malloc(Q * sizeof(unsigned char*));
    unsigned char **SD = malloc(Q * sizeof(unsigned char*));
    unsigned char **II = malloc(Q * sizeof(unsigned char*));
    unsigned char **ID = malloc(Q * sizeof(unsigned char*));
    for (i = 0; i < Q; ++i) {
        SI[i] = malloc(Q * sizeof(unsigned char));
        SD[i] = malloc(Q * sizeof(unsigned char));
        II[i] = malloc(Q * sizeof(unsigned char));
        ID[i] = malloc(Q * sizeof(unsigned char));
    }

    /* copiar cuadrantes desde MatTotal */
    for (i = 0; i < Q; ++i)
        for (j = 0; j < Q; ++j) {
            SI[i][j] = MatTotal[i][j];
            SD[i][j] = MatTotal[i][j + Q];
            II[i][j] = MatTotal[i + Q][j];
            ID[i][j] = MatTotal[i + Q][j + Q];
        }

    /* liberar MatTotal (cada fila y luego el array de punteros) */
    for (i = 0; i < N; ++i) free(MatTotal[i]);
    free(MatTotal);

    /* filas solicitadas */
    int filas[] = {0, 1, 255, 256, 511};
    
    for (i = 0; i < 5; ++i) {
        int r = filas[i];
        if (r < 0 || r >= N) {
            printf("Fila %d fuera de rango -> ignorada.\n", r);
            continue;
        }

        double sum = 0.0, sumsq = 0.0;
        for (c = 0; c < N; ++c) {
            unsigned char val;
            if (r < Q) {               /* mitad superior */
                if (c < Q) val = SI[r][c];
                else       val = SD[r][c - Q];
            } else {                   /* mitad inferior */
                int rr = r - Q;
                if (c < Q) val = II[rr][c];
                else       val = ID[rr][c - Q];
            }
            double dv = val;
            sum += dv;
            sumsq += dv * dv;
        }
        double mean = sum / N;
        double var = sumsq / N - mean * mean;
        double sd = sqrt(var);
        printf("Desviacion estandar fila %d: %.6f\n", r, sd);
    }

    /* liberar cuadrantes */
    for (i = 0; i < Q; ++i) {
        free(SI[i]); free(SD[i]); free(II[i]); free(ID[i]);
    }
    free(SI); free(SD); free(II); free(ID);

    return 0;
}

