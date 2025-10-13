#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#define generar 100000
#define rango 10

long double aleatorioX(long double min, long double max);
long double aleatorioY(int min, int max);

int main(int argc, char *argv[]) {

    //Variables MPI
    int np, yo;

    //Variable para almacenar cuantos números caen dentro de la integral
    int hit = 0;

    //Variable para almacenar el f(x) generado en cada iteración
    int f;

    //Número e de referencia (long double sólo 18 dígitos de precisión)
    long double e = 2.7182818284590452353602874713526624977572471L;

    //m es la altura del rectángulo en el que se inscribe la integral
    int h = 4;

    //Inicialización de MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &yo);

    //Se calcula n, la anchura del rectángulo
    long double n = (long double)rango / np;

    //Se calcula el dominio de trabajo del nodo
    long double min = n*yo;
    long double max = n*(yo+1);

    //Se inicia el generador de números aleatorios, cada nodo una semilla diferente
    srand(yo);

    //Bucle que genera puntos aleatorios y registra si están dentro de la función
    for(int i = 0; i < generar; i++){
        long double x = aleatorioX(min, max);
        long double y = aleatorioY(0, h);
        f = powl(x, e) / tgammal(x + 1.0L);
        if(y <= f){hit++;}
    }

    printf("Soy el nodo %d [%LF, %LF)y han entrado %d puntos\n", yo, min, max, hit);

    MPI_Finalize();
    return 0;
}

//Genera un número aleatorio [min, max), para las coordenadas x
long double aleatorioX(long double min, long double max) {
    long double r = (long double)drand48();
    return min + r * (max - min);
}

//Genera un número aleatorio [min, max], para las coordenadas y
long double aleatorioY(int min, int max) {
    long double r = (long double)rand() / (long double)RAND_MAX;
    return min + r * (max - min);
}

