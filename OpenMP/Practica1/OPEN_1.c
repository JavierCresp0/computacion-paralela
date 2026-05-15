#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* genera un double aleatorio en (-0.01, 0.01) */
double rand_offdiag() {
    return ((double)rand() / (double)RAND_MAX * 2.0 - 1.0) * 0.01;
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Uso: %s N m omega matrix_file output_text_file\n", argv[0]);
        return 1;
    }

    /* Parametros de entrada */
    int N = atoi(argv[1]);
    int m = atoi(argv[2]);
    double omega = atof(argv[3]);
    const char *matriz_file = argv[4];
	int nhilos = atoi(argv[5]);
    double beta = 1.0 - omega;

    /* Reservas */
    double **M;
    double *xk = malloc((size_t)N * sizeof(double));
    double *xk1 = malloc((size_t)N * sizeof(double));
    double *vec1 = malloc((size_t)N * sizeof(double));  
	double *vec2 = malloc((size_t)N * sizeof(double));
    double *vecNormas = malloc((size_t)(m + 1) * sizeof(double));
	int i, j, k;
    double norma, suma,  total_sum, local_sum, start, end, tiempo;              

    M = malloc(N * sizeof(double *));
    for (i = 0; i < N; i++) {
        M[i] = malloc(N * sizeof(double));
    }

    /* lectura/creación de la matriz */
    FILE *f = fopen(matriz_file, "rb");
    if (f) {
        printf("Fichero leido \n");
        for (i = 0; i < N; ++i) {
            fread(M[i], sizeof(double), (size_t)N, f);
        }
        fclose(f);
    } else {
        printf("Fichero aleatorio\n");
        srand((unsigned)time(NULL));
        for (i = 0; i < N; ++i)
            for (j = 0; j < N; ++j)
                M[i][j] = (i == j) ? 1.0 : rand_offdiag();
        f = fopen(matriz_file, "wb");
        for (i = 0; i < N; ++i)
            fwrite(M[i], sizeof(double), N, f);
        fclose(f);
    }

    /* inicializar xk y xk1*/
    for (i = 0; i < N; ++i) {
        xk[i] = 1.0;
        xk1[i] = 0.0;
    }

	/* ----------------------------- */
	/*--- PRIMERAS 3 ITERACIONES ---*/
	/* ----------------------------- */

	start = omp_get_wtime();
	#pragma omp parallel num_threads(nhilos) \
			shared(N, omega, beta, M, xk, xk1, vecNormas, norma, total_sum) \
			private(i, j, k, local_sum) default(none)
	{
		int nh = omp_get_num_threads();
		int iam = omp_get_thread_num();
		int ini = (iam * N) / nh;
		int fin = ((iam + 1) * N) / nh;

		/* acumulador global */
		#pragma omp single 
		{
			total_sum = 0.0;
		}

		/* Norma inicial */
		local_sum = 0.0;
		for (i = ini; i < fin; i++) {
			local_sum += xk[i] * xk[i];
		}

		/* añadir su parte al acumulador global */
		#pragma omp critical
		{
			total_sum += local_sum;
		}		
		#pragma omp barrier
		
		#pragma omp single
		{
			vecNormas[0] = sqrt(total_sum);
			norma = vecNormas[0];
			total_sum = 0.0;
		}

		/* Hacemos las primeras 3 iteraciones  */
		for (k = 0; k < 3; ++k) {
			for (i = ini; i < fin; i++) {
				double suma_local = 0.0;
				for (j = 0; j < N; ++j) {
					suma_local += M[i][j] * xk[j];
				}
				xk1[i] = suma_local * omega + beta * xk[i];
			}
			/* esperar a que todos terminen de escribir xk1 */
			#pragma omp barrier
			
			local_sum = 0.0;
			/* division y cuadrados: cada hilo su parte */
			for (i = ini; i < fin; i++) {
				xk[i] = xk1[i] / norma;
				local_sum += xk[i] * xk[i];
			}
			#pragma omp barrier

			/* añadir al acumulador global */
			#pragma omp critical
			{
				total_sum += local_sum;
			}
			#pragma omp barrier
			
			#pragma omp single nowait
			{
				norma = sqrt(total_sum);
				vecNormas[k + 1] = norma;
				total_sum = 0.0;
			}
		}
	} 

	/* ----------------------------- */
    /*---- RESTO DE ITERACIONES -----*/
    /* ----------------------------- */
	
	for (k = 0; k < m-3; ++k) {
		
		/* Calculamos w*M·xk */
		#pragma omp parallel num_threads(nhilos) shared(N, omega, M, xk, vec1, suma) private(i, j) default(none)
		{					
			#pragma omp for schedule(static) reduction(+:suma) nowait
			for (i = 0; i < N; ++i) {
				suma = 0.0;
				for (j = 0; j < N; ++j) {
					suma += M[i][j] * xk[j];
				}
				vec1[i] = suma * omega;
			}
			
		}
		
		/* Calculamos 1-w * xk */
		#pragma omp parallel num_threads(nhilos) shared(N, beta, xk, vec2) private(i) default(none)
		{					
			#pragma omp for schedule(static) nowait
			for (i = 0; i < N; i++){		
				vec2[i] = beta * xk[i]; 
			}				
		}
		
		/* Calculamos la suma y la division */
		#pragma omp parallel num_threads(nhilos) \
				shared(N, xk1, xk, norma, vec1, vec2) private(i) default(none)
		{			
			#pragma omp for schedule(static) nowait
			for (i = 0; i < N; i++){		
				xk1[i] = vec1[i] + vec2[i]; 
				xk[i] = xk1[i] / norma; 
			}	
		}
		
		/* calcular norma de xk */
		#pragma omp parallel num_threads(nhilos) shared(N, xk, xk1, vecNormas, norma, local_sum, k) private(i) default(none)
		{            
            #pragma omp single nowait
            {
                local_sum = 0.0;
            }

            #pragma omp for reduction(+:local_sum) schedule(static)	
            for (i = 0; i < N; ++i) {
                local_sum += xk[i] * xk[i];
            }

            #pragma omp single nowait
            {
                norma = sqrt(local_sum);
                vecNormas[k+4] = norma;
            }
		}
	}
	
	end= omp_get_wtime();
	tiempo = end - start;	
	printf("\nTiempo con %i hilos: %f segundos (start: %g end: %g)\n",nhilos, tiempo,start,end);
		
	/* Crear fichero */
	FILE *out = fopen("NormasR.txt", "w");	
	for(i=0;i<= m;i++){
		fprintf(out, "%d: %.6e\n",i,vecNormas[i]);
	}
	fclose(out);
	
	/* liberar memoria */
	for (i = 0; i < N; ++i) free(M[i]);
    free(M);
    free(xk);
    free(xk1);
    free(vec1);
	free(vec2);
	free(vecNormas);
	
    return 0;
}

