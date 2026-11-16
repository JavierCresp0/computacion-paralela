#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *input_bin = "datos10dobles.bin";
    const char *out_dobles_linea = "double10.txt";
    const char *out_dobles_lineas15 = "double15.txt";
    const char *out_enteros_txt = "enteros.txt";
    const char *out_enteros_bin = "enteros_reescritos.bin";

    FILE *fin = fopen(input_bin, "rb");
    if (!fin) { perror("No se puede abrir datos10dobles.bin"); return EXIT_FAILURE; }

    /* ---- Leer 10 doubles ---- */
    double d[10];
    if (fread(d, sizeof(double), 10, fin) != 10) {
        fprintf(stderr, "Error: datos10dobles.bin no contiene 10 doubles.\n");
        fclose(fin);
        return EXIT_FAILURE;
    }

    /* escribir una línea con 10 decimales separados por ';' */
    FILE *f1 = fopen(out_dobles_linea, "w");
    if (!f1) { perror("No se puede crear dobles_linea.txt"); fclose(fin); return EXIT_FAILURE; }
    for (size_t i = 0; i < 10; ++i) {
        if (i) putc(';', f1);
        fprintf(f1, "%.10f", d[i]);
    }
    fclose(f1);

    /* escribir cada double en su propia línea con 15 decimales */
    FILE *f2 = fopen(out_dobles_lineas15, "w");
    if (!f2) { perror("No se puede crear dobles_lineas15.txt"); fclose(fin); return EXIT_FAILURE; }
    for (size_t i = 0; i < 10; ++i) {
        fprintf(f2, "%.15f\n", d[i]);
    }
    fclose(f2);

    /* ---- Rebobinar ---- */
    rewind(fin);

    FILE *f3 = fopen(out_enteros_txt, "w");
    if (!f3) { perror("No se puede crear enteros_separados.txt"); fclose(fin); return EXIT_FAILURE; }
    FILE *fb = fopen(out_enteros_bin, "wb");
    if (!fb) { perror("No se puede crear enteros_reescritos.bin"); fclose(f3); fclose(fin); return EXIT_FAILURE; }

    int val;
    int primero = 1;
    size_t leidos = 0;
    while (fread(&val, sizeof(int), 1, fin) == 1) {
        if (!primero) putc(';', f3);
        fprintf(f3, "%d", val);
        fwrite(&val, sizeof(int), 1, fb);
        primero = 0;
        ++leidos;
    }

    fclose(f3);
    fclose(fb);
    fclose(fin);

    printf("Hecho. Leídos %zu enteros. Generados:\n  %s\n  %s\n  %s\n  %s\n",
           leidos, out_dobles_linea, out_dobles_lineas15, out_enteros_txt, out_enteros_bin);

    return EXIT_SUCCESS;
}