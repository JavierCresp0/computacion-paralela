#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Programa simple para leer una matriz cuadrada de unsigned char desde un
   fichero binario, almacenarla como puntero doble, y calcular:
   - media de cada fila (vector dinámico)
   - media de las columnas pares (0,2,4,...)
   - desviación estándar de filas: 0,1,2,100,N-3,N-2 (si existen)
   - máximo y mínimo de las medias de todas las filas y de todas las columnas

   NOTA: por simplicidad no se comprueban los retornos de malloc/fopen/fread.
*/

int main(int argc, char *argv[]) {
    const char *filename = "fichero1.bin";
    if (argc > 1) filename = argv[1];

    /* Abrir fichero binario */
    FILE *f = fopen(filename, "rb");

    /* Determinar tamaño del fichero */
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Calcular N (matriz N x N). Redondeamos sqrt para seguridad. */
    int N = (int)(sqrt((double)filesize) + 0.5);
    printf("La matriz mide %d x %d \n", N, N);

    /* Reservar matriz con puntero doble, fila por fila */
    unsigned char **mat = malloc(N * sizeof(unsigned char *));
    for (int i = 0; i < N; ++i) {
        mat[i] = malloc(N * sizeof(unsigned char));
    }

    /* --- Leer directamente fila a fila en mat[i] --- */    
    for (int i = 0; i < N; ++i) {
        fread(mat[i], sizeof(unsigned char), N, f);
    }
    fclose(f);
    /* ------------------------------------------------------- */

    /* Vector de medias de filas */
    double *row_mean = malloc(N * sizeof(double));
    for (int i = 0; i < N; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) sum += (double)mat[i][j];
        row_mean[i] = sum / (double)N;
    }

    /* Vector de medias de columnas pares (0,2,4,...) */
    int even_count = (N + 1) / 2; /* columnas 0,2,... hasta <N */
    double *even_col_mean = malloc(even_count * sizeof(double));
    for (int k = 0; k < even_count; ++k) {
        int col = 2 * k;
        double sum = 0.0;
        for (int i = 0; i < N; ++i) sum += (double)mat[i][col];
        even_col_mean[k] = sum / (double)N;
    }

    /* Media de todas las columnas (para luego sacar max/min) */
    double *col_mean = malloc(N * sizeof(double));
    for (int j = 0; j < N; ++j) {
        double sum = 0.0;
        for (int i = 0; i < N; ++i) sum += (double)mat[i][j];
        col_mean[j] = sum / (double)N;
    }

    /* Desviación estándar para filas 0,1,2,100,N-3,N-2 */
    int query_indices[6] = {0, 1, 2, 100, N - 3, N - 2};
    printf("Desviaciones estandar (filas solicitadas):\n");
    for (int q = 0; q < 6; ++q) {
        int r = query_indices[q];
        if (r >= 0 && r < N) {
            double mean = row_mean[r];
            double acc = 0.0;
            for (int j = 0; j < N; ++j) {
                double diff = (double)mat[r][j] - mean;
                acc += diff * diff;
            }
            double variance = acc / (double)N; /* desviación poblacional */
            double stddev = sqrt(variance);
            printf("  Fila %d: media = %.6f, desviacion estandar = %.6f\n", r, mean, stddev);
        } else {
            printf("  Fila %d: fuera de rango (N = %d), se omite\n", r, N);
        }
    }

    /* Máximo y mínimo global de la media de todas las filas */
    double max_row_mean = row_mean[0], min_row_mean = row_mean[0];
    for (int i = 1; i < N; ++i) {
        if (row_mean[i] > max_row_mean) max_row_mean = row_mean[i];
        if (row_mean[i] < min_row_mean) min_row_mean = row_mean[i];
    }
    printf("\nMedia de filas: max = %.6f, min = %.6f\n", max_row_mean, min_row_mean);

    /* Máximo y mínimo global de la media de todas las columnas */
    double max_col_mean = col_mean[0], min_col_mean = col_mean[0];
    for (int j = 1; j < N; ++j) {
        if (col_mean[j] > max_col_mean) max_col_mean = col_mean[j];
        if (col_mean[j] < min_col_mean) min_col_mean = col_mean[j];
    }
    printf("Media de columnas: max = %.6f, min = %.6f\n", max_col_mean, min_col_mean);

    /* Mostrar vector de medias de filas y medias de columnas pares 
    printf("\nMedias de filas (por fila, 0..N-1):\n");
    for (int i = 0; i < N; ++i) {
        printf("  fila %d: %.6f\n", i, row_mean[i]);
    }

    printf("\nMedias de columnas pares (columnas 0,2,4,...):\n");
    for (int k = 0; k < even_count; ++k) {
        int col = 2 * k;
        printf("  columna %d: %.6f\n", col, even_col_mean[k]);
    } */

    /* Liberar memoria */
    for (int i = 0; i < N; ++i) free(mat[i]);
    free(mat);
    free(row_mean);
    free(even_col_mean);
    free(col_mean);

    return 0;
}