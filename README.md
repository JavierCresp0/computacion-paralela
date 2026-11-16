# ⚡ Computación Paralela · MPI & OpenMP en C

Prácticas de la asignatura **Computación Paralela** (3º curso) sobre paralelización de algoritmos en **C** con dos paradigmas distintos:

- 🔵 **MPI** — paralelismo de **memoria distribuida** (paso de mensajes entre procesos)
- 🟠 **OpenMP** — paralelismo de **memoria compartida** (hilos sobre un mismo proceso)

Cada práctica incluye un análisis de **eficiencia** y **speedup** comparando la versión paralela con la secuencial.

## 📁 Estructura

El repositorio sigue el orden de la asignatura: primero la base en C, después las versiones secuenciales y por último su paralelización.

```
Practica0/       ← base en C: ficheros binarios, memoria dinámica y punteros
Secuencial/
├── Practica1/   ← iteración matriz-vector con cálculo de normas
└── Practica2/   ← filtrado de imágenes (media y media ponderada) e histogramas

MPI/
├── Practica1/   ← introducción a MPI: comunicación punto a punto
└── Practica2/   ← filtrado de imágenes y comparación de histogramas

OpenMP/
├── Practica1/   ← directivas básicas, paralelización de bucles
└── Practica2/   ← filtrado de imágenes y comparación de histogramas
```

Las carpetas `MPI/` y `OpenMP/` contienen el código fuente `.c` y un informe `Eficiencia.pdf` con mediciones y análisis.

### Práctica 0 — C para computación científica

| Archivo | Qué trabaja |
|---------|-------------|
| `tarea1_ficheros_binarios.c` | Lectura y escritura de `double` e `int` en ficheros binarios y de texto |
| `tarea2_memoria_dinamica.c` | Escalares, vectores con `malloc` y matrices con puntero doble |
| `tarea3_reduccion_matriz.c` | Eliminar filas y columnas de una matriz leída de un fichero binario |
| `tarea4_aritmetica_punteros.c` | La misma reducción usando solo aritmética de punteros |
| `tarea5_estadisticas_matriz.c` | Medias, desviación estándar, máximos y mínimos por filas y columnas |
| `tarea6_column_major.c` | Matriz en un único bloque contiguo, almacenada por columnas |
| `tarea7_orden_recorrido.c` | Cómo cambia el resultado al recorrer un vector en sentido creciente o decreciente |
| `tarea8_cuadrantes.c` | División de una matriz en cuatro cuadrantes |

Se incluyen los dos ficheros de entrada pequeños (`datos10dobles.bin` y `f80bytes.bin`); las matrices grandes de prueba no se versionan.

### Versiones secuenciales

Son el punto de partida de las prácticas paralelas y la referencia para medir el *speedup*:

- `Secuencial/Practica1/iteracion_matriz_vector.c` — repite `m` veces el producto matriz-vector con un factor de relajación `omega`, normalizando el vector y guardando la norma de cada iteración.
- `Secuencial/Practica2/filtrado_imagen.c` — aplica a una imagen `.raw` un filtro de media y otro de media ponderada (3×3, con extensión simétrica en los bordes), calcula los histogramas y compara cuál la modifica más.

## 🛠️ Tecnologías

![C](https://img.shields.io/badge/C-00599C?style=flat&logo=c&logoColor=white)
![OpenMP](https://img.shields.io/badge/OpenMP-FF6B00?style=flat)
![MPI](https://img.shields.io/badge/MPI-005571?style=flat)

## 🎯 Conceptos aplicados

- Paralelismo de memoria distribuida (MPI)
- Paralelismo de memoria compartida (OpenMP)
- Comunicación punto a punto y colectiva (`MPI_Send`, `MPI_Recv`, `MPI_Bcast`, `MPI_Reduce`)
- Directivas OpenMP (`#pragma omp parallel`, `for`, `reduction`, `critical`)
- Sincronización y condiciones de carrera
- Métricas: **speedup**, **eficiencia** y **ley de Amdahl**

## 🚀 Compilación y ejecución

### Práctica 0 y versiones secuenciales
```bash
gcc Practica0/tarea1_ficheros_binarios.c -o tarea1 -lm
gcc Secuencial/Practica1/iteracion_matriz_vector.c -o secuencial1 -lm
./secuencial1 100 50 0.7 matriz.bin normas.txt    # N, iteraciones, omega, matriz, salida
```

### MPI
```bash
mpicc MPI_1.c -o mpi_1
mpirun -np 4 ./mpi_1
```

### OpenMP
```bash
gcc -fopenmp OPEN_1.c -o open_1
./open_1
```

---

📌 *Prácticas del 3º curso del Grado en Ingeniería Informática — Universidad Miguel Hernández.*
