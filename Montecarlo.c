#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>

#define rango 1

long double aleatorio(long double min, long double max);

int main(int argc, char *argv[]) {

    /*
     * Obtenemos por argumentos el número total de puntos y la versión del programa a usar
     * version 0: Todos los nodos le envían sus resultados al nodo 0
     * version 1: Los nodos impares le envían sus resultados al nodo par anterior, y luego estos al 0. (Disponible a partir de np = 4)
     * version 2: Los nodos envían sus datos al múltiplo de 4 anterior, y luego estos al 0. (Disponible a partir de np = 8)
    */
    long generar = 1000000;
    int version = 0;
    if(argc == 2){
        generar = atol(argv[1]);
        version = 0;
    }else if(argc > 2){
        generar = atol(argv[1]);
        version = atoi(argv[2]);
    }

    //Variables MPI
    int np, yo;

    //Variable para almacenar cuantos números caen dentro de la integral
    int hit = 0;

    //Variable para almacenar el f(x) generado en cada iteración
    long double f;

    //Número e de referencia (long double sólo 18 dígitos de precisión)
    long double e = 2.7182818284590452353602874713526624977572471L;

    //Inicialización de MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &yo);

    //Se toma el tiempo inicial
    long double inicio = MPI_Wtime();

    //Se calcula n, la anchura del rectángulo
    long double n = (long double)rango / np;

    //Se calcula el dominio de trabajo del nodo
    long double min = n*yo;
    long double max = n*(yo+1);

    //Se inicia el generador de números aleatorios, cada nodo una semilla diferente
    srand48((yo+1)*time(0));

    //Bucle que genera puntos aleatorios y registra si están dentro de la función
    for(long i = 0; i < generar; i++){
        long double x = aleatorio(min, max);
        long double y = aleatorio(0, e);
        f = powl(e, x);
        if(y <= f){hit++;}
    }

    long double eParcial = ((hit*e)/(generar*np));

    //Version 2
    if(version == 2 && np >= 8){

        if(yo%4 != 0){
            MPI_Send(&eParcial, 1, MPI_LONG_DOUBLE, yo-(yo%4), yo, MPI_COMM_WORLD);

        }else{
            long double recv = 0;
            long double suma = eParcial;
            MPI_Status status;

            int num_recv = (np - yo - 1 >= 3) ? 3 : (np - yo - 1);
            if (num_recv < 0) num_recv = 0;
            for(int i = 0; i < num_recv; i++){
                MPI_Recv(&recv, 1, MPI_LONG_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                suma += recv;
            }

            if(yo != 0){
                MPI_Send(&suma, 1, MPI_LONG_DOUBLE, 0, yo, MPI_COMM_WORLD);

            }else{
                for(int i = 1; i < np/4; i++){
                    MPI_Recv(&recv, 1, MPI_LONG_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    suma += recv;
                }
                printf("El valor aproximado de e es %LF\n", suma+1);
            }
        }

    //Version 1
    }else if(version == 1 && np >= 4){

        if(yo%2 == 1){
            MPI_Send(&eParcial, 1, MPI_LONG_DOUBLE, yo-1, yo, MPI_COMM_WORLD);

        }else{
            long double recv = 0;
            long double suma = eParcial;
            MPI_Status status;
            MPI_Recv(&recv, 1, MPI_LONG_DOUBLE, yo+1, yo+1, MPI_COMM_WORLD, &status);
            suma += recv;

            if(yo != 0){
                MPI_Send(&suma, 1, MPI_LONG_DOUBLE, 0, yo, MPI_COMM_WORLD);

            }else{
                for(int i = 1; i < np/2; i++){
                    MPI_Recv(&recv, 1, MPI_LONG_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    suma += recv;
                }
                printf("El valor aproximado de e es %LF\n", suma+1);
            }
        }
    //version 0
    }else{

        if(yo != 0){
            MPI_Send(&eParcial, 1, MPI_LONG_DOUBLE, 0, yo, MPI_COMM_WORLD);

        }else{
            long double recv = 0;
            long double suma = eParcial;
            MPI_Status status;
            for(int i = 1; i < np; i++){
                MPI_Recv(&recv, 1, MPI_LONG_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                suma += recv;
            }
            printf("El valor aproximado de e es %LF\n", suma+1);
        }
    }

    //Se toma el tiempo final
    long double fin = MPI_Wtime();
    printf("Tiempo de ejecución del nodo %d: %LF segundos\n", yo, fin - inicio);

    MPI_Finalize();

    return 0;
}

//Genera un número aleatorio [min, max), para las coordenadas x
long double aleatorio(long double min, long double max) {
    long double r = (long double)drand48();
    return min + r * (max - min);
}

