#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char *argv[]) {
    int i, j, ii, jj;
    if (argc < 2) {
        printf("Uso: %s imagen.raw\n", argv[0]);
        return 0;
    }
    const char *infile = argv[1];

    /*Calcular tamaño  */
    FILE *f = fopen(infile, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    /* calcular N  */
    int N = sqrt(fsize);     

    /* reservar matrices */    
    unsigned char **orig = malloc((N+2) * sizeof(unsigned char *));
    unsigned char **mean = malloc(N * sizeof(unsigned char *));
    unsigned char **wmean = malloc(N * sizeof(unsigned char *));
    for (i = 0; i < N+2; ++i) orig[i] = malloc((N+2) * sizeof(unsigned char));
    for (i = 0; i < N; ++i) {
        mean[i]  = malloc(N * sizeof(unsigned char));
        wmean[i] = malloc(N * sizeof(unsigned char));
    }

    /* leer  */
    for (i = 1; i <= N; ++i) {
        fread(orig[i] + 1, sizeof(unsigned char), N, f);
    }
    fclose(f);

    /* --- EXTENSIÓN SIMÉTRICA  --- */   
    for (j = 1; j <= N; ++j) {
        orig[0][j] = orig[2][j];     
        orig[N+1][j] = orig[N-1][j];   
    }
    
    for (i = 0; i <= N+1; ++i) {
        orig[i][0] = orig[i][2];
        orig[i][N+1] = orig[i][N-1];
    }

    /* --- Convolución  --- */    
    for (i = 1; i <= N; ++i) {
        for (j = 1; j <= N; ++j) {
            int sum = 0;
            for (ii = -1; ii <= 1; ++ii){
                for (jj = -1; jj <= 1; ++jj){
                    sum += orig[i + ii][j + jj];
                    mean[i-1][j-1] = (sum / 9);
                }
            }

            /* media ponderada */
            int sum2 = 0;
            sum2 += orig[i-1][j-1] * 1;
            sum2 += orig[i-1][j  ] * 2;
            sum2 += orig[i-1][j+1] * 1;

            sum2 += orig[i  ][j-1] * 2;
            sum2 += orig[i  ][j  ] * 4;
            sum2 += orig[i  ][j+1] * 2;

            sum2 += orig[i+1][j-1] * 1;
            sum2 += orig[i+1][j  ] * 2;
            sum2 += orig[i+1][j+1] * 1;

            wmean[i-1][j-1] = (sum2 / 16);
        }
    }

    /* histogramas */
    long hist_orig[256] = {0}, hist_mean[256] = {0}, hist_wmean[256] = {0};
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            
            hist_orig[ orig[i+1][j+1] ]++;
            hist_mean[ mean[i][j] ]++;
            hist_wmean[ wmean[i][j] ]++;
        }
    }

    /* Distancia euclídea */
    double dist_mean = 0.0, dist_wmean = 0.0;
    for (i = 0; i < 256; ++i) {
        double d1 = hist_orig[i] - hist_mean[i];
        double d2 = hist_orig[i] - hist_wmean[i];
        dist_mean  += d1 * d1;
        dist_wmean += d2 * d2;
    }
    dist_mean  = sqrt(dist_mean);
    dist_wmean = sqrt(dist_wmean);

    /* Guardar imágenes */
    FILE *fo = fopen("Media.raw", "wb");
    for (i = 0; i < N; ++i) fwrite(mean[i], sizeof(unsigned char), N, fo);
    fclose(fo);

    FILE *fw = fopen("MediaPonderada.raw", "wb");
    for (i = 0; i < N; ++i) fwrite(wmean[i], sizeof(unsigned char), N, fw);
    fclose(fw);

    /* Guardar distancias e indicar cuál modificó más */
    FILE *fd = fopen("distancias.txt", "w");
    fprintf(fd, "Distancia euclidea entre el original y la media: %f\n", dist_mean);
    fprintf(fd, "Distancia euclidea entre el original y la media ponderada: %f\n", dist_wmean);
    if (dist_mean > dist_wmean){
        fprintf(fd, "El filtrado que modifico en mayor medida el histograma: MEDIA\n");
    }else{
        fprintf(fd, "El filtrado que modifico en mayor medida el histograma: MEDIA PONDERADA\n");
    }         
    fclose(fd);

    /* Por pantalla */
    printf("Distancia con media= %f\n  Distancia con media ponderada = %f\n", dist_mean, dist_wmean);

    /* liberar memoria */
    for (i = 0; i < N+2; ++i) free(orig[i]);
    for (i = 0; i < N; ++i) {
        free(mean[i]);
        free(wmean[i]);
    }
    free(orig);
    free(mean);
    free(wmean);

    return 0;
}