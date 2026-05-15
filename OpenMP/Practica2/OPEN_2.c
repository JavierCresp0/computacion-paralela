#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

int main(int argc, char *argv[]) {
    
    if (argc < 3) {
        printf("Uso: %s imagen.raw\n", argv[0]);
        return 0;
    }
	
	/* Declarar */
	int i, j, ii, jj;
    const char *infile = argv[1];
	int nhilos = atoi(argv[2]);
	double start, end, tiempo;
	long hist_orig[256] = {0}, hist_mean[256] = {0}, hist_wmean[256] = {0};
	double dist_mean = 0.0, dist_wmean = 0.0;

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
	
	
	start = omp_get_wtime();
    /* --- Filtrado  --- */  
	#pragma omp parallel num_threads(nhilos) \
			shared(N,mean,orig,wmean) default(none)
	{		
		int i, j, ii, jj;
		int nh = omp_get_num_threads();
		int iam = omp_get_thread_num();
		int ini = ((iam * N) / nh)+1;
		int fin = ((iam + 1) * N) / nh;
		int sum,sum2;
		
		/* --- Convolucion  --- */    
		for (i = ini; i <= fin; ++i) {
			for (j = 1; j <= N; ++j) {
				sum = 0;
				for (ii = -1; ii <= 1; ++ii){
					for (jj = -1; jj <= 1; ++jj){
						sum += orig[i + ii][j + jj];
						mean[i-1][j-1] = (sum / 9);
					}
				}

				/* media ponderada */
				sum2 = 0;
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
	}
	
	/* histogramas */
	#pragma omp parallel num_threads(nhilos) \
		shared(N,mean,orig,wmean,hist_mean,hist_orig,hist_wmean) default(none)
	{
		int i,j; 
		long local_orig[256] = {0}, local_mean[256] = {0}, local_wmean[256] = {0};
		int nh = omp_get_num_threads();
		int iam = omp_get_thread_num();
		int ini = (iam * N) / nh;
		int fin = ((iam + 1) * N) / nh;
		
		/* histogramas */		
		for (i = ini; i < fin; ++i) {
			for (j = 0; j < N; ++j) {
				
				local_orig[ orig[i+1][j+1] ]++;
				local_mean[ mean[i][j] ]++;
				local_wmean[ wmean[i][j] ]++;
			}
		}
		
		/* Combinar local -> global (sección corta, protegida) */
		#pragma omp critical
		{
			for (int k = 0; k < 256; ++k) {
				hist_orig[k]  += local_orig[k];
				hist_mean[k]  += local_mean[k];
				hist_wmean[k] += local_wmean[k];
			}
		}
	}	
	
	/* Distancia euclídea */
	#pragma omp parallel num_threads(nhilos) \
		shared(dist_mean,dist_wmean,wmean,hist_mean,hist_orig,hist_wmean) default(none)
	{
		int i;
		double d1,d2;
		/* Distancia euclídea */
		#pragma omp for reduction(+:dist_mean,dist_wmean) schedule(static)	
		for (i = 0; i < 256; ++i) {
			d1 = hist_orig[i] - hist_mean[i];
			d2 = hist_orig[i] - hist_wmean[i];
			dist_mean  += d1 * d1;
			dist_wmean += d2 * d2;
		}
		#pragma omp single nowait
        {
			dist_mean  = sqrt(dist_mean);
			dist_wmean = sqrt(dist_wmean);        
        }

	}
	end= omp_get_wtime();
	tiempo = end - start;	
	printf("\nTiempo con %i hilos: %f segundos (start: %g end: %g)\n",nhilos, tiempo,start,end);
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
