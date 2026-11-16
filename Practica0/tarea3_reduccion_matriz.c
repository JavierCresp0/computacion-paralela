#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    const char *infile = argv[1];

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

    /* copiar: por cada fila conservada, copiar cada elemento conservado con memcpy */
    int rdst = 0;
    for (int i = 0; i < N; ++i) {
        if (remR[i]) continue;
        int *src = A[i];
        int *dst = MatRed[rdst];
        int pos = 0;
        for (int j = 0; j < N; ++j) {
            if (!remC[j]) {
                memcpy(&dst[pos], &src[j], sizeof(int));
                pos++;
            }
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

    /* guardar MatRed */
    FILE *fm = fopen(outMat, "wb");
    for (int i = 0; i < keptR; ++i){
        fwrite(MatRed[i], sizeof(int), keptC, fm);
    } 
    fclose(fm);

    /* crear VecRed y guardar */
    int total = keptR * keptC;
    int *VecRed = malloc(total * sizeof(int));
    for (int i = 0; i < keptR; ++i) memcpy(VecRed + i * keptC, MatRed[i], keptC * sizeof(int));
    FILE *fv = fopen(outVec, "wb");
    fwrite(VecRed, sizeof(int), total, fv);
    fclose(fv);

    /* imprimir primeras 10 filas x 10 columnas (si no hay tantas, imprime lo que haya) */
    int prow = (N < 10) ? N : 10;
    int pcol = (N < 10) ? N : 10;
    printf("\n--- Primeras %d filas x %d cols (ORIGINAL) ---\n", prow, pcol);
    for (int i = 0; i < prow; ++i) {
        printf("fila %2d: ", i);
        for (int j = 0; j < pcol; ++j) {
            printf("%d", A[i][j]);
            if (j+1 < pcol) putchar(' ');
        }
        if (N > pcol) printf("  ...");
        putchar('\n');
    }

    int rprow = (keptR < 10) ? keptR : 10;
    int rpcol = (keptC < 10) ? keptC : 10;
    printf("\n--- Primeras %d filas x %d cols (REDUCIDA) ---\n", rprow, rpcol);
    for (int i = 0; i < rprow; ++i) {
        printf("rfil %2d: ", i);
        for (int j = 0; j < rpcol; ++j) {
            printf("%d", MatRed[i][j]);
            if (j+1 < rpcol) putchar(' ');
        }
        if (keptC > rpcol) printf("  ...");
        putchar('\n');
    }

    /* liberar (mínimo) */
    free(VecRed);
    for (int i = 0; i < keptR; ++i) free(MatRed[i]);
    free(MatRed);
    free(remR); free(remC);
    for (int i = 0; i < N; ++i) free(A[i]);
    free(A);
    return 0;
}
