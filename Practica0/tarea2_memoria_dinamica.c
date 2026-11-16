#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *f = fopen("datos10dobles.bin", "rb");
    if (!f) {
        perror("Error al abrir archivo");
        return 1;
    }

    /* 1) Leer un dato (escalar) 10 veces */
    printf("1) Leer un dato por vez (escalares):\n");
    for (int i = 0; i < 10; ++i) {
        double x;
        if (fread(&x, sizeof(double), 1, f) != 1) {
            fprintf(stderr, "Error leyendo dato %d\n", i);
            fclose(f);
            return 1;
        }
        printf("[%d] = %f\n", i, x);
    }

    /* 2) Leer los 10 datos de una sola vez en un vector dinámico */
    rewind(f);
    printf("\n2) Leer los 10 datos en un vector dinámico (malloc):\n");
    double *vec = (double *)malloc(10 * sizeof(double));
    if (!vec) {
        fprintf(stderr, "malloc falló\n");
        fclose(f);
        return 1;
    }
    if (fread(vec, sizeof(double), 10, f) != 10) {
        fprintf(stderr, "Error leyendo los 10 datos\n");
        free(vec);
        fclose(f);
        return 1;
    }
    for (int i = 0; i < 10; ++i) {
        printf("[%d] = %f\n", i, vec[i]);
    }
    free(vec);

    /* --- Punto 3: matriz 5x2, reservar fila a fila y leer fila a fila --- */
    {
        int rows = 5, cols = 2;
        double **matriz5x2 = (double **)malloc(rows * sizeof(double *));
        
        for (int i = 0; i < rows; ++i) {
            matriz5x2[i] = (double *)malloc(cols * sizeof(double));

        }

        /* leer fila a fila (5 fread) */
        for (int i = 0; i < rows; ++i) {
            fread(matriz5x2[i], sizeof(double), cols, f);   
        }

        /* mostrar */
        printf("Matriz 5x2:\n");
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                printf("m52[%d][%d] = %f\n", i, j, matriz5x2[i][j]);

        for (int i = 0; i < rows; ++i) free(matriz5x2[i]);
        free(matriz5x2);
    }

    rewind(f);

    /* --- Punto 4: matriz 2x5, reservar fila a fila y leer fila a fila --- */
    {
        int rows = 2, cols = 5;
        double **matriz2x5 = (double **)malloc(rows * sizeof(double *));
        
        for (int i = 0; i < rows; ++i) {
            matriz2x5[i] = (double *)malloc(cols * sizeof(double));
        }

        /* leer fila a fila (2 fread) */
        for (int i = 0; i < rows; ++i) {
            fread(matriz2x5[i], sizeof(double), cols, f);   
        }

        /* mostrar */
        printf("Matriz 2x5:\n");
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                printf("m25[%d][%d] = %f\n", i, j, matriz2x5[i][j]);

        for (int i = 0; i < rows; ++i) free(matriz2x5[i]);
        free(matriz2x5);
    }

    fclose(f);
    return 0;
}