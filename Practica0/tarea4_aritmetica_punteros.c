#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    const char *infile = argv[1];

    if (infile == NULL) {
        printf("Uso: %s fichero1.bin|fichero2.bin (pasa el nombre del fichero de entrada)\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(infile, "rb");

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    long nelem = fsize / (long)sizeof(int);
    int N = (int)(sqrt((double)nelem) + 0.5);

    /* leer fila a fila en int** (malloc por fila) */
    int **A = malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        A[i] = malloc(N * sizeof(int));
        fread(A[i], sizeof(int), N, f);
    }
    fclose(f);

    /* marcar filas y columnas a eliminar */
    bool *remR = calloc(N, sizeof(bool));
    bool *remC = calloc(N, sizeof(bool));

    int rows_to_remove[] = {0,1,2,3,4,7,9};
    for (int k = 0; k < (int)(sizeof(rows_to_remove)/sizeof(rows_to_remove[0])); ++k) {
        int idx = rows_to_remove[k];
        if (0 <= idx && idx < N) remR[idx] = true;
    }
    for (int t = 1; t <= 3; ++t) {
        int idx = N - t;
        if (0 <= idx && idx < N) remR[idx] = true;
    }

    int cols_to_remove[] = {0,3,4,6};
    for (int k = 0; k < (int)(sizeof(cols_to_remove)/sizeof(cols_to_remove[0])); ++k) {
        int idx = cols_to_remove[k];
        if (0 <= idx && idx < N) remC[idx] = true;
    }
    for (int t = 1; t <= 3; ++t) {
        int idx = N - t;
        if (0 <= idx && idx < N) remC[idx] = true;
    }

    /* contar conservadas */
    int keptR = 0, keptC = 0;
    for (int i = 0; i < N; ++i) if (!remR[i]) ++keptR;
    for (int j = 0; j < N; ++j) if (!remC[j]) ++keptC;

    /* crear MatRed (malloc por fila) */
    int **MatRed = malloc(keptR * sizeof(int*));
    for (int i = 0; i < keptR; ++i) MatRed[i] = malloc(keptC * sizeof(int));

    /* COPIA SIN memcpy NI INDICES: usar solo aritmética de punteros y * */
    int rdst = 0;
    for (int i = 0; i < N; ++i) {
        if (remR[i]) continue;
        int *psrc = A[i];        /* pointer que recorre la fila origen */
        int *pdst = MatRed[rdst];/* pointer que escribe la fila destino */
        for (int j = 0; j < N; ++j) {
            if (!remC[j]) {
                /* asignación mediante aritmética de punteros y * */
                *pdst = *psrc;
                pdst++;        /* avanza en la fila reducida */
            }
            psrc++;            /* avanza en la fila origen (siempre) */
        }
        ++rdst;
    }

    /* nombres de salida según fichero de entrada */
    char outMat[64], outVec[64];
    if (strstr(infile, "fichero1.bin") != NULL) {
        strcpy(outMat, "matrizreducida1.bin"); strcpy(outVec, "vectorreducido1.bin");
    } else if (strstr(infile, "fichero2.bin") != NULL) {
        strcpy(outMat, "matrizreducida2.bin"); strcpy(outVec, "vectorreducido2.bin");
    } else {
        strcpy(outMat, "matrizreducida.bin"); strcpy(outVec, "vectorreducido.bin");
    }

    /* guardar MatRed: se puede usar fwrite por filas */
    FILE *fm = fopen(outMat, "wb");
    for (int i = 0; i < keptR; ++i){
        fwrite(MatRed[i], sizeof(int), keptC, fm);
    }
    fclose(fm);

    /* crear VecRed y rellenarlo SIN usar índices (solo punteros y *) */
    int total = keptR * keptC;
    int *VecRed = malloc(total * sizeof(int));
    int *pvec = VecRed;
    for (int i = 0; i < keptR; ++i) {
        int *pread = MatRed[i];
        for (int k = 0; k < keptC; ++k) {
            *pvec = *pread; /* asignación mediante punteros */
            pvec++;
            pread++;
        }
    }

    /* guardar VecRed */
    FILE *fv = fopen(outVec, "wb");
    fwrite(VecRed, sizeof(int), total, fv);
    fclose(fv);

    /* imprimir dimensiones (simple) */
    printf("N original = %d\n", N);
    printf("Dimensiones reducidas: filas=%d, cols=%d (total=%d)\n", keptR, keptC, total);

    /* liberar (mínimo) */
    free(VecRed);
    for (int i = 0; i < keptR; ++i) free(MatRed[i]);
    free(MatRed);
    free(remR); free(remC);
    for (int i = 0; i < N; ++i) free(A[i]);
    free(A);
    return 0;
}