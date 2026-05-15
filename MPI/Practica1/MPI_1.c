#include  "mpi.h"
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
        printf("Ejemplo: %s 100 50 0.7 matriz.bin normas.txt\n", argv[0]);
        return 1;
    }

	/* Declarar */
	int nproces, myrank, i, k, h, err, len;
	int N,m,j,base,resto,myFilas,myIndice;
	MPI_Status status;
	MPI_Init(&argc,&argv); 
    MPI_Comm_size(MPI_COMM_WORLD,&nproces);
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	const char *matriz_file;
	const char *output_file;	
	double omega,norma, sum, beta, tmp, suma_global_xk1, start, end, tiempo;
	double **M;
	double **myMatriz;
	double *xk, *xk1, *vecNormas;

	/* lectura */ 
	if(myrank == 0){
		N = atoi(argv[1]);
		m = atoi(argv[2]);
		omega = atof(argv[3]);
		matriz_file = argv[4];
		output_file = argv[5];
	}
	
	/* Reserva */
	int *numFilas = malloc(nproces * sizeof(int));
	int *numIndices = malloc(nproces * sizeof(int));
	if(myrank == 0){
		M = malloc(N * sizeof(double *));
		for (int i = 0; i < N; ++i) {
			M[i] = malloc(N * sizeof(double));
		}
		vecNormas = malloc((m+1) * sizeof(double));
	}
	
	/* Intentar leer fichero */ 
	if(myrank == 0){
		FILE *f = fopen(matriz_file, "rb");
		if (f) {			
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
			f = fopen(matriz_file, "wb");
			for (int i = 0; i < N; ++i)
				fwrite(M[i], sizeof(double), N, f);
			fclose(f);
		}
	}

	/* Mandar  N, omega y m */
	if(myrank == 0){	
		for (i=1;i<nproces;i++){			
			MPI_Send(&N,1,MPI_INT,i,10,MPI_COMM_WORLD);		
			MPI_Send(&omega,1,MPI_DOUBLE,i,20,MPI_COMM_WORLD);
			MPI_Send(&m,1,MPI_INT,i,50,MPI_COMM_WORLD);
		}			
	}else{
		MPI_Recv(&N,1,MPI_INT,0,10,MPI_COMM_WORLD,&status);	
		MPI_Recv(&omega,1,MPI_DOUBLE,0,20,MPI_COMM_WORLD,&status);
		MPI_Recv(&m,1,MPI_INT,0,50,MPI_COMM_WORLD,&status);			
    }

	/* Calculo numFilas y numIndices */
	base = N/nproces;
	resto = N % nproces;
	for(i=0;i<nproces;i++){
		numFilas[i]= base;
		numIndices[i] = (i==0) ? 0 : numIndices[i-1] + base;
		if(i == nproces-1) numFilas[i] += resto;
	}
	
	/* Cada proceso ahora toma su porción */
	myFilas  = numFilas[myrank];
	myIndice = numIndices[myrank];
	printf("Soy el proceso %d y tengo %d filas; indice inicial %d\n", myrank, myFilas, myIndice);
	
	/* inicializar myMatriz */
	if(myrank != 0){
		myMatriz = malloc(myFilas * sizeof(double *));
		for (int i = 0; i < myFilas; ++i) {
			myMatriz[i] = malloc(N * sizeof(double));
		}
	}
	
	/* Inicializar xk y xk1 */
	xk1 = malloc(N * sizeof(double));
	xk = malloc(N * sizeof(double));
	
	/* 	Mandar matriz */
	if (myrank == 0) {
		for (i = 1; i < nproces; i++) {
			int start = numIndices[i];
			int end   = start + numFilas[i]; 
			for (j = start; j < end; j++) {
				MPI_Send(M[j], N, MPI_DOUBLE, i, 15, MPI_COMM_WORLD); 
			}
		}
	}else{
		for (i=0;i<myFilas;i++){
			MPI_Recv(myMatriz[i],N,MPI_DOUBLE,0,15,MPI_COMM_WORLD,&status);			
		}
    }
	
	/* ----------------------------------------------------------------- */
	/* --------- YA TENEMOS LOS DATOS | EMPEZAMOS LOS CALCULOS --------- */
	/* ----------------------------------------------------------------- */
	
	if(myrank == 0){		
		for (i = 0; i < N; ++i) xk[i] = 1.0;
		for (i = 0; i < N; ++i) xk1[i] = 0;
	}
	
	/* mandar xk y xk1 */
	if(myrank == 0){
		for (i=1;i<nproces;i++){	
			MPI_Send(xk,N,MPI_DOUBLE,i,25,MPI_COMM_WORLD);
			MPI_Send(xk1,N,MPI_DOUBLE,i,35,MPI_COMM_WORLD);
		}		
	}else{
        MPI_Recv(xk,N,MPI_DOUBLE,0,25,MPI_COMM_WORLD,&status);
		MPI_Recv(xk1,N,MPI_DOUBLE,0,35,MPI_COMM_WORLD,&status);
    }	
		
	/* Calculo de la norma */	
	norma = 0.0;
	beta = 1-omega;
	
	/* ALLREDUCE */
	for(int i = 0; i < myFilas; i++){
		norma += xk[myIndice+i]*xk[myIndice+i];	
	}
	
	double suma_global = 0.0;
    if (myrank == 0) {
        suma_global = norma;
        for (int src = 1; src < nproces; ++src) {            
            MPI_Recv(&tmp, 1, MPI_DOUBLE, src, 30, MPI_COMM_WORLD, &status);
            suma_global += tmp;
        }
        /* enviar la suma global a todos los demás */
        for (int dest = 1; dest < nproces; ++dest) {
            MPI_Send(&suma_global, 1, MPI_DOUBLE, dest, 31, MPI_COMM_WORLD);
        }
    } else {
        /* enviar mi contribución al root */
        MPI_Send(&norma, 1, MPI_DOUBLE, 0, 30, MPI_COMM_WORLD);
        /* recibir la suma global desde root */
        MPI_Recv(&suma_global, 1, MPI_DOUBLE, 0, 31, MPI_COMM_WORLD, &status);
    }
	/* ALLREDUCE */
	
	norma = sqrt(suma_global);
	if (myrank == 0){
		vecNormas[0] = norma;
	}
	/* ----------------------------- */
	/*--- PRIMERAS 2 ITERACIONES ---*/
	/* ----------------------------- */
	if(myrank==0){
		if(myrank==0) start=MPI_Wtime();
		myMatriz=M;			
	}
	for(k = 0; k < 2; k++){
		/* Calculamos w*M·xk */
		for (i = 0; i < myFilas; i++){
			sum = 0.0;
			for (j = 0; j < N; j++) {
					sum += myMatriz[i][j] * xk[j];				
			}
			xk1[i] = sum * omega;  
		}
		
		/* Calculamos 1-w * xk */
		for (i = 0; i < myFilas; i++){		
			xk1[i] += beta * xk[myIndice + i]; 
			xk1[i] = xk1[i] / norma;
		}		
			
		/* --- Norma de xk1 para siguiente iteracion --- */
		norma = 0.0;

		/* solo suma local */
		for (i = 0; i < myFilas; ++i) {
			norma += xk1[i] * xk1[i];
		}

		suma_global_xk1 = 0.0;

		/* Allreduce */
		if (myrank == 0) {
			suma_global_xk1 = norma;
			for (int src = 1; src < nproces; ++src) {
				MPI_Recv(&tmp, 1, MPI_DOUBLE, src, 40, MPI_COMM_WORLD, &status);
				suma_global_xk1 += tmp;
			}
			/* enviar la suma global a todos los demás */
			for (int dest = 1; dest < nproces; ++dest) {
				MPI_Send(&suma_global_xk1, 1, MPI_DOUBLE, dest, 41, MPI_COMM_WORLD);
			}
		} else {
			/* envío mi parte al 0 */
			MPI_Send(&norma, 1, MPI_DOUBLE, 0, 40, MPI_COMM_WORLD);
			/* recibo la suma global */
			MPI_Recv(&suma_global_xk1, 1, MPI_DOUBLE, 0, 41, MPI_COMM_WORLD, &status);
		}
		/* Allreduce */
		
		/* norma  */
		norma = sqrt(suma_global_xk1);
		if (myrank == 0){
			vecNormas[k+1] = norma;
		}
		
		/* MPI_Allgatherv  */		
		for (h = 0; h < myFilas; h++)
			xk[myIndice + h] = xk1[h];

		for (int p = 0; p < nproces; p++) {
			if (p == myrank) continue;

			if (myrank < p) {
				MPI_Send(xk1, myFilas, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
				MPI_Recv(&xk[numIndices[p]], numFilas[p], MPI_DOUBLE, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			} else {
				MPI_Recv(&xk[numIndices[p]], numFilas[p], MPI_DOUBLE, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Send(xk1, myFilas, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
			}
		}
		/* MPI_Allgatherv  */		
	}
	
	/* ----------------------------- */
	/*--- RESTO DE ITERACIONES ---*/
	/* ----------------------------- */
	
	for (k=0; k < m-2; k++){
		/* Calculamos w*M·xk */
		for (i = 0; i < myFilas; i++){
			sum = 0.0;
			for (j = 0; j < N; j++) {
				sum += myMatriz[i][j] * xk[j];			
			}
			xk1[i] = sum * omega;  
		}
		
		/* Calculamos 1-w * xk */
		for (i = 0; i < myFilas; i++){		
			xk1[i] += beta * xk[myIndice + i]; 
			xk1[i] = xk1[i] / norma;
		}		
			
		/* --- Norma de xk1 para siguiente iteracion --- */
		norma = 0.0;

		/* solo suma local  */
		for (i = 0; i < myFilas; ++i) {
			norma += xk1[i] * xk1[i];
		}	

		/* Norma de xk1 */
		norma =0;		
		for (i = 0; i < myFilas; ++i) { 
			norma += xk1[i] * xk1[i];  
		} 
		
		MPI_Allreduce(&norma, &norma, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 
		norma = sqrt(norma); 
		if (myrank == 0){
			vecNormas[k+3] = norma;
		}
		MPI_Allgatherv(xk1, myFilas, MPI_DOUBLE, xk, numFilas, numIndices, MPI_DOUBLE, MPI_COMM_WORLD);
	}
	
	if(myrank==0){
		end=MPI_Wtime();
		tiempo = end - start;	
		printf("\nTiempo con %i procesos: %f segundos (start: %g end: %g)\n",nproces, tiempo,start,end);		
	} 

	if (myrank == 0){
		FILE *out = fopen("NormasR.txt", "w");	
		for(i=0;i<= m;i++){
			fprintf(out, "%d: %.10e\n",i,vecNormas[i]);
		}
		fclose(out);
	}
	
	/* Liberar memoria  */
    free(xk);
    free(xk1);
    free(numFilas);
    free(numIndices);
    if (myrank == 0) {        
        for (i = 0; i < N; ++i) {
            if (M[i]) free(M[i]);
        }
    free(M);        
    free(vecNormas);
    } else {
        for (i = 0; i < myFilas; ++i) {
            free(myMatriz[i]);
        }
        free(myMatriz);
        
    }
	
	MPI_Finalize();
	return 0;
}
	
	

	

	
