# ⚡ Computación Paralela · MPI & OpenMP en C

Prácticas de la asignatura **Computación Paralela** (3º curso) sobre paralelización de algoritmos en **C** con dos paradigmas distintos:

- 🔵 **MPI** — paralelismo de **memoria distribuida** (paso de mensajes entre procesos)
- 🟠 **OpenMP** — paralelismo de **memoria compartida** (hilos sobre un mismo proceso)

Cada práctica incluye un análisis de **eficiencia** y **speedup** comparando la versión paralela con la secuencial.

## 📁 Estructura

```
MPI/
├── Practica1/   ← introducción a MPI: comunicación punto a punto
└── Practica2/   ← filtrado de imágenes y comparación de histogramas

OpenMP/
├── Practica1/   ← directivas básicas, paralelización de bucles
└── Practica2/   ← filtrado de imágenes y comparación de histogramas
```

Cada carpeta contiene:
- Código fuente `.c`
- Informe `Eficiencia.pdf` con mediciones y análisis

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
