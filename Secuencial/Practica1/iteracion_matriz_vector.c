#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* genera un double aleatorio en (-0.01, 0.01) */
double rand_offdiag() {
    return ((double)rand() / (double)RAND_MAX * 2.0 - 1.0) * 0.01;
}

/* calcula y devuelve la norma euclídea de un vector de tamaño n */
double norma_euclidea(const double *v, int n) {
    double s = 0.0;
    for (int i = 0; i < n; ++i) s += v[i] * v[i];
    return sqrt(s);
}

/* multiplicación matriz-vector (M es double** con filas M[i]) */
void matvec_mul(double **M, const double *x, double *y, int n) {
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += M[i][j] * x[j];
        }
        y[i] = sum;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Uso: %s N m omega matrix_file output_text_file\n", argv[0]);
        printf("Ejemplo: %s 100 50 0.7 matriz.bin normas.txt\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int m = atoi(argv[2]);
    double omega = atof(argv[3]);
    const char *matrix_file = argv[4];
    const char *out_file = argv[5];


    if (!(omega > 0.0 && omega < 1.0)) {
        printf("omega debe ser estrictamente > 0 y < 1\n");
        return 1;
    }
    

    /* reservar memoria: array de punteros + cada fila */
    double **M = malloc(N * sizeof(double *));
    for (int i = 0; i < N; ++i) {
        M[i] = malloc(N * sizeof(double));
    }

    double *xk = malloc(N * sizeof(double));
    double *Mx = malloc(N * sizeof(double));
    double *v  = malloc(N * sizeof(double));

    /* intentar leer fichero que contiene solo N*N doubles*/
    FILE *f = fopen(matrix_file, "rb");
    if (f) {   
        /* leer fila a fila */
        for (int i = 0; i < N; ++i) {
            fread(M[i], sizeof(double), (size_t)N, f); 
        }
        fclose(f);
       
    } else {
        /* fichero no existe: generarlo y guardarlo */
        srand((unsigned)time(NULL));
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                M[i][j] = (i == j) ? 1.0 : rand_offdiag();
        f = fopen(matrix_file, "wb");
        for (int i = 0; i < N; ++i)
            fwrite(M[i], sizeof(double), N, f);
        fclose(f);
    }

    /* x0 = (1,1,...,1) */
    for (int i = 0; i < N; ++i) xk[i] = 1.0;

    FILE *out = fopen(out_file, "w");
    double norm_xk = norma_euclidea(xk, N);
    fprintf(out, "0 %.15g\n", norm_xk);
    
    double beta = 1.0 - omega;
    for (int k = 0; k < m; ++k) {
        matvec_mul(M, xk, Mx, N);
   
        for (int i = 0; i < N; ++i){
            v[i] = omega * Mx[i] + beta * xk[i];
            xk[i] = v[i] / norm_xk;
        }     
        norm_xk = norma_euclidea(xk, N);
        fprintf(out, "%d %.15e\n", k+1, norm_xk);
    }

    fclose(out);

    /* liberar memoria: cada fila y luego el array de punteros */
    for (int i = 0; i < N; ++i) free(M[i]);
    free(M);
    free(xk);
    free(Mx);
    free(v);

    printf("Hecho: normas escritas 2.0 en '%s'\n", out_file);
    return 0;
}