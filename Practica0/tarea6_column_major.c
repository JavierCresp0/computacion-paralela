#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
    const char *filename = "fichero1.bin";
    if (argc > 1) filename = argv[1];

    FILE *f = fopen(filename, "rb");

    /* determinar tamaño y N */
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    int N = (int)(sqrt((double)filesize) + 0.5);
    printf("La matriz mide %d x %d\n", N, N);

    /* reservar matriz en column-major: mat[col*N + row] */
    unsigned char *mat = malloc((size_t)N * (size_t)N * sizeof(unsigned char));

    /* leer elemento a elemento desde el fichero */
    unsigned char x;
    for (int i = 0; i < N; ++i) {         /* por cada fila i (archivo está por filas) */
        for (int j = 0; j < N; ++j) {     /* por cada columna j en la fila i */
            fread(&x, sizeof(unsigned char), 1, f); /* leer 1 byte */
            mat[j * N + i] = x;           /* colocar en column-major */
        }
    }
    fclose(f);

    /* calcular medias por fila */
    double *row_mean = malloc(N * sizeof(double));
    for (int i = 0; i < N; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) sum += (double) mat[j * N + i];
        row_mean[i] = sum / (double)N;
    }

    /* medias por columna */
    double *col_mean = malloc(N * sizeof(double));
    for (int j = 0; j < N; ++j) {
        double sum = 0.0;
        int base = j * N;
        for (int i = 0; i < N; ++i) sum += (double) mat[base + i];
        col_mean[j] = sum / (double)N;
    }

    /* medias de columnas pares (0,2,4,...) */
    int even_count = (N + 1) / 2;
    double *even_col_mean = malloc(even_count * sizeof(double));
    for (int k = 0; k < even_count; ++k) {
        int col = 2 * k;
        even_col_mean[k] = col_mean[col];
    }

    /* desviaciones estándar para filas 0,1,2,100,N-3,N-2 (con comprobación de rango) */
    int query_indices[6] = {0, 1, 2, 100, N - 3, N - 2};
    printf("Desviaciones estandar (filas solicitadas):\n");
    for (int q = 0; q < 6; ++q) {
        int r = query_indices[q];
        
        double mean = row_mean[r];
        double acc = 0.0;
        for (int j = 0; j < N; ++j) {
            double diff = (double)mat[j * N + r] - mean;
            acc += diff * diff;
        }
        double variance = acc / (double)N; /* varianza poblacional */
        double stddev = sqrt(variance);
        printf("  Fila %d: media = %.6f, desviacion estandar = %.6f\n", r, mean, stddev);

    }

    /* máximo y mínimo global de medias de filas */
    double max_row_mean = row_mean[0], min_row_mean = row_mean[0];
    for (int i = 1; i < N; ++i) {
        if (row_mean[i] > max_row_mean) max_row_mean = row_mean[i];
        if (row_mean[i] < min_row_mean) min_row_mean = row_mean[i];
    }
    printf("\nMedia de filas: max = %.6f, min = %.6f\n", max_row_mean, min_row_mean);

    /* máximo y mínimo global de medias de columnas */
    double max_col_mean = col_mean[0], min_col_mean = col_mean[0];
    for (int j = 1; j < N; ++j) {
        if (col_mean[j] > max_col_mean) max_col_mean = col_mean[j];
        if (col_mean[j] < min_col_mean) min_col_mean = col_mean[j];
    }
    printf("Media de columnas: max = %.6f, min = %.6f\n", max_col_mean, min_col_mean);

    /* liberar memoria */
    free(mat);
    free(row_mean);
    free(col_mean);
    free(even_col_mean);

    return 0;
}