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
	int nproces, myrank,i, j, ii, jj, sum, sum2, N;
	MPI_Status status;
	MPI_Init(&argc,&argv); 
    MPI_Comm_size(MPI_COMM_WORLD,&nproces);
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	unsigned char **orig,**mean,**wmean, **myMatriz;
	int base, resto;
	int *numFilas, *numIndices;
	int myFilas, myIndice;
	
	/* leer parametro */
	const char *infile = argv[1];
	FILE *f = NULL;

    /*Calcular tamaño  */
	if(myrank == 0){
		f = fopen(infile, "rb");
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		rewind(f);
		N = sqrt(fsize);
	}

    /* reservar matrices */
	if(myrank == 0){    
		orig = malloc((N+2) * sizeof(unsigned char *));
		mean = malloc(N * sizeof(unsigned char *));
		wmean = malloc(N * sizeof(unsigned char *));
		for (i = 0; i < N+2; ++i) orig[i] = malloc((N+2) * sizeof(unsigned char));
		for (i = 0; i < N; ++i) {
			mean[i]  = malloc(N * sizeof(unsigned char));
			wmean[i] = malloc(N * sizeof(unsigned char));
		}	
	}

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

	/* Manodar N */
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	/* Calculo numFilas y numIndices */
	numFilas   = malloc(nproces * sizeof(int));
	numIndices = malloc(nproces * sizeof(int));
	base = N/nproces;
	resto = N % nproces;
	for(i=0;i<nproces;i++){
		numFilas[i]= base;
		numIndices[i] = (i==0) ? 1 : numIndices[i-1] + base;
		if(i == nproces-1) numFilas[i] += resto;
	}
	
	/* Cada proceso ahora toma su porción */
	myFilas  = numFilas[myrank]+2;
	myIndice = numIndices[myrank];
	printf("Soy el proceso %d y tengo %d filas; indice inicial %d\n", myrank, myFilas, myIndice);
	
	/* inicializar myMatriz */
	if(myrank != 0){
		myMatriz = malloc(myFilas * sizeof(unsigned char *));
		for (int i = 0; i < myFilas; ++i) {
			myMatriz[i] = malloc(N * sizeof(unsigned char	));
		}
	}
	
	/* 	Mandar matriz */
	if (myrank == 0) {
		for (i = 1; i < nproces; i++) {
			int start = numIndices[i];
			int end   = start + numFilas[i]; 
			printf("La matriz del proceso %d empieza en el indice %d start y termina en %d excluye (-1) \n",i,start,end);
			for (j = start; j < end; j++) {
				MPI_Send(orig[j], N, MPI_UNSIGNED_CHAR, i, 15, MPI_COMM_WORLD); 
			}
		}
	}else{ 	
		printf("Soy el proceso %d y quiero mi matriz de tamaño %d X %d \n",myrank,myFilas,N);
		for (i=0;i<myFilas;i++){
			MPI_Recv(myMatriz[i],N,MPI_UNSIGNED_CHAR,0,15,MPI_COMM_WORLD,&status);			
		}
    }
	
	//

	MPI_Finalize();
    return 0;
}