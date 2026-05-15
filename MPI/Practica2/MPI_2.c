#include  "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char *argv[]) {    
    if (argc < 2) {
        printf("Uso: %s imagen.raw\n", argv[0]);
        return 0;
    }
	
	/* Declarar */
	int nproces, myrank,i, r, j, ii, jj, sum, sum2, N, err;
	MPI_Status status;
	MPI_Init(&argc,&argv); 
    MPI_Comm_size(MPI_COMM_WORLD,&nproces);
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	unsigned char **orig,**mean,**wmean, **myMatriz;
	unsigned char **meanT,**wmeanT;
	int base, resto, start, end, outRows, myIndiceDis;
	int *numFilas, *numIndices;
	int myFilas, myIndice;
	const char *infile;
	FILE *f, *fo, *fw, *fd;
	double fsize,dist_mean, dist_wmean, dist_mean_local, dist_wmean_local, d1, d2,startT,endT,tiempo, myDistancia;	
	long hist_local_orig[256] = {0}, hist_local_mean[256] = {0}, hist_local_wmean[256] = {0};
	long hist_orig[256] = {0}, hist_mean[256] = {0}, hist_wmean[256] = {0};
	
	/* leer parametro */
	if(myrank == 0){
		infile = argv[1];
	}
		
    /*Calcular tamaño  */
	if(myrank == 0){
		f = fopen(infile, "rb");
		fseek(f, 0, SEEK_END);
		fsize = ftell(f);
		rewind(f);
		N = sqrt(fsize);
	}

    /* reservar matrices */
	if(myrank == 0){    
		orig = malloc((N+2) * sizeof(unsigned char *));

		for (i = 0; i < N+2; ++i) orig[i] = malloc((N+2) * sizeof(unsigned char));

	}
	/* Reserva de vectores */
	numFilas = malloc(nproces * sizeof(int));
	numIndices = malloc(nproces * sizeof(int));


    /* leer  */
	if(myrank == 0){
		for (i = 1; i <= N; ++i) {
			fread(orig[i] + 1, sizeof(unsigned char), N, f);
		}
		fclose(f);
	}
	
    /* --- EXTENSIÓN SIMÉTRICA  --- */   
	if(myrank == 0){
		for (j = 1; j <= N; ++j) {
			orig[0][j] = orig[2][j];     
			orig[N+1][j] = orig[N-1][j];   
		}	
		for (i = 0; i <= N+1; ++i) {
			orig[i][0] = orig[i][2];
			orig[i][N+1] = orig[i][N-1];
		}
	}

	/* Mandar N */
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	/* Calculo numFilas y numIndices */
	base = N/nproces;
	resto = N % nproces;
	for(i=0;i<nproces;i++){
		numFilas[i]= base;
		numIndices[i] = (i==0) ? 0 : numIndices[i-1] + base;
		if(i == nproces-1) numFilas[i] += resto;
	}
	
	/* Cada proceso ahora toma su porción */
	myFilas  = numFilas[myrank]+2;
	myIndice = numIndices[myrank];
	outRows = myFilas - 2; 
	printf("Soy el proceso %d y tengo %d filas; indice inicial %d\n", myrank, myFilas, myIndice);
	
	/* Reserva de matrices */
	if(myrank!=0){
		mean = malloc(outRows * sizeof(unsigned char *));
		wmean = malloc(outRows * sizeof(unsigned char *));
		for (i = 0; i < outRows; ++i) {
				mean[i]  = malloc(N * sizeof(unsigned char));
				wmean[i] = malloc(N * sizeof(unsigned char));
		}	
	}
	
	/* inicializar myMatriz */
	if(myrank != 0){
		myMatriz = malloc(myFilas * sizeof(unsigned char *));
		for (int i = 0; i < myFilas; ++i) {
			myMatriz[i] = malloc((N+2) * sizeof(unsigned char	));
		}
	}
	
	/* inicializar meanT y meanT */
	if (myrank == 0) {
		meanT = malloc(N * sizeof(unsigned char *));
		wmeanT = malloc(N * sizeof(unsigned char *));
		for (i = 0; i < N; ++i) {
			meanT[i]  = malloc(N * sizeof(unsigned char));
			wmeanT[i] = malloc(N * sizeof(unsigned char));
		}
	}
	
	/* 	Mandar matriz */
	if (myrank == 0) {
		myMatriz = orig;
		mean = meanT;
		wmean = wmeanT;
		for (i = 1; i < nproces; i++) {
			start = numIndices[i]+1;
			end   = start + numFilas[i]; 
			//printf("La matriz del proceso %d empieza en el indice %d start y termina en %d inclusiva \n",i,start,end);
			for (j = start-1; j <= end; j++) {
				MPI_Send(orig[j], N+2, MPI_UNSIGNED_CHAR, i, 15, MPI_COMM_WORLD); 
			}
		}
	}else{ 	
		//printf("Soy el proceso %d y quiero mi matriz de tamaño %d X %d \n",myrank,myFilas,N+2);
		for (i=0;i<myFilas;i++){
			MPI_Recv(myMatriz[i],N+2,MPI_UNSIGNED_CHAR,0,15,MPI_COMM_WORLD,&status);			
		}
    }
	
	/* Calculo myDistancia y myIndiceDis*/
    base = 256 / nproces;
    resto  = 256 % nproces;
    myDistancia;
	myIndiceDis;
	myDistancia = base;
	if (myrank == nproces-1) myDistancia += resto;
	myIndiceDis = base*myrank;
	
	dist_mean_local = 0.0;
	dist_wmean_local= 0.0;
	dist_mean= 0.0;
	dist_wmean= 0.0;
	
	/* Tiempo */
	if(myrank==0) startT=MPI_Wtime();
	
	/* --- Convolución  --- */    
    for (i = 1; i <= outRows; ++i) {
        for (j = 1; j <= N; ++j) {
            sum = 0;
            for (ii = -1; ii <= 1; ++ii){
                for (jj = -1; jj <= 1; ++jj){
                    sum += myMatriz[i + ii][j + jj];
                    mean[i-1][j-1] = (sum / 9);
                }
            }

            /* media ponderada */
            sum2 = 0;
            sum2 += myMatriz[i-1][j-1] * 1;
            sum2 += myMatriz[i-1][j  ] * 2;
            sum2 += myMatriz[i-1][j+1] * 1;

            sum2 += myMatriz[i  ][j-1] * 2;
            sum2 += myMatriz[i  ][j  ] * 4;
            sum2 += myMatriz[i  ][j+1] * 2;

            sum2 += myMatriz[i+1][j-1] * 1;
            sum2 += myMatriz[i+1][j  ] * 2;
            sum2 += myMatriz[i+1][j+1] * 1;

            wmean[i-1][j-1] = (sum2 / 16);
        }
    }

    /* histogramas */
	for (i = 0; i < outRows; ++i) {
		for (j = 0; j < N; ++j) {
			hist_orig[myMatriz[i + 1][j + 1]]++;
			hist_mean[mean[i][j]]++;
			hist_wmean[wmean[i][j]]++;
		}
	}

	/* Reduction segura */
	MPI_Allreduce(MPI_IN_PLACE, hist_orig, 256, MPI_LONG, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(MPI_IN_PLACE, hist_mean, 256, MPI_LONG, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(MPI_IN_PLACE, hist_wmean, 256, MPI_LONG, MPI_SUM, MPI_COMM_WORLD);	
	
	/* Juntar Matrices */
	/* El resto envía sus filas */		  
	if (myrank != 0) {
		for (i = 0; i < outRows; ++i) {
			MPI_Send(mean[i],  N, MPI_UNSIGNED_CHAR, 0, 100, MPI_COMM_WORLD);
			MPI_Send(wmean[i], N, MPI_UNSIGNED_CHAR, 0, 101, MPI_COMM_WORLD);
		}
	}
	
	/* recibe las filas */
	if (myrank == 0) {		
		for (i = 1; i < nproces; i++) {
			start = numIndices[i];   
			end  = numFilas[i];    
			for (r = 0; r < end; r++) {
				MPI_Recv(meanT[start + r], N, MPI_UNSIGNED_CHAR, i, 100, MPI_COMM_WORLD, &status);
				MPI_Recv(wmeanT[start + r], N, MPI_UNSIGNED_CHAR, i, 101, MPI_COMM_WORLD, &status);
			}
		}
	}	
	
	/* Distancia euclidea */
	end = myDistancia + myIndiceDis;
	for (i = myIndiceDis; i < end; ++i) {
		d1 = hist_orig[i] - hist_mean[i];
		d2 = hist_orig[i] - hist_wmean[i];
		dist_mean_local  += d1 * d1;
		dist_wmean_local += d2 * d2;
	}	
	MPI_Reduce(&dist_mean_local, &dist_mean, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(&dist_wmean_local, &dist_wmean, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if(myrank == 0){
		dist_mean  = sqrt(dist_mean);
		dist_wmean = sqrt(dist_wmean);
		endT =MPI_Wtime();
		tiempo = endT - startT;	
		printf("\nTiempo con %i procesos: %f segundos (start: %g end: %g)\n",nproces, tiempo,startT,endT);	
	}
	
	/* Cosas finales */
	if(myrank == 0){
		/* Guardar imágenes */	
		fo = fopen("Media.raw", "wb");
		for (i = 0; i < N; ++i) fwrite(meanT[i], sizeof(unsigned char), N, fo);
		fclose(fo);

		fw = fopen("MediaPonderada.raw", "wb");
		for (i = 0; i < N; ++i) fwrite(wmeanT[i], sizeof(unsigned char), N, fw);
		fclose(fw);

		/* Guardar distancias e indicar cuál modificó más */
		fd = fopen("distancias.txt", "w");
		fprintf(fd, "Distancia euclidea entre el original y la media: %f\n", dist_mean);
		fprintf(fd, "Distancia euclidea entre el original y la media ponderada: %f\n", dist_wmean);
		if (dist_mean > dist_wmean){
			fprintf(fd, "El filtrado que modificó en mayor medida el histograma: MEDIA\n");
		}else{
			fprintf(fd, "El filtrado que modificó en mayor medida el histograma: MEDIA PONDERADA\n");
		}         
		fclose(fd);

		/* Por pantalla */
		printf("Distancia con media= %f\n  Distancia con media ponderada = %f\n", dist_mean, dist_wmean);
	}
	
	/* liberar memoria */
	if (myrank == 0) {
		for (i = 0; i < N + 2; ++i) free(orig[i]);
		free(orig);
		for (i = 0; i < N; ++i) free(meanT[i]);
		free(meanT);
		for (i = 0; i < N; ++i) free(wmeanT[i]);
		free(wmeanT);
    } else {
        for (i = 0; i < myFilas; ++i) free(myMatriz[i]);
        free(myMatriz);
        for (i = 0; i < outRows; ++i) free(mean[i]);
        free(mean);
        for (i = 0; i < outRows; ++i) free(wmean[i]);
        free(wmean);
    }
	free(numFilas);
	free(numIndices);
	
	MPI_Finalize();
	return 0;
}
