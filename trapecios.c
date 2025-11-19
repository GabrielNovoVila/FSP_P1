#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>

// Declaración de variables globales
long double e=2.7182818284590452353602874713526624977572471;
long double pi=3.1415926535897932384626433832795028841971694;
long double min=0;
long double max=1;
long double eCalculado=0;
long double eDefinitivo;
long double h;
long double valor_fijo;
long double suma;
long double valor;
long double miParte;
long double error;
double inicio;
double final;

int divisiones, metodo;


//4 por integral de 0 a 1 de sqrt(1-x²)

int main(int argc, char *argv[]) {

    // Inicialización de MPI
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Cogemos el número de divisiones y la lógica de envío de mensajes por argumento
    if(argc!=3){
        if (rank == 0){
            printf("\nUso: mpirun -np N programa <metodo> <numero>\n");
            printf("\nMetodo es un número:\n");
            printf("0: Todos los procesos le mandan su cálculo al 0, quien los sumará todos\n");
            printf("1: Cada proceso le manda su cálculo al anterior, que, a su vez,\n\t los mandarán al 0, quien los sumará todos\n");
            printf("2: Cada grupo de 4 procesos lo mandan al primero de los mismos, que, a su vez, \n\t los mandarán al 0, quien los sumará todos\n");

        } 
        MPI_Finalize();
        return 0;
    }
        
    metodo=atoi(argv[1]);
    divisiones=atoi(argv[2]);

    long double datos_recibidos[size];

    switch(metodo){
        
        // Todos los procesos le mandan su suma al 0
        case 0:
            // Comenzamos a medir el tiempo de la ejecución de cada proceso

            inicio=MPI_Wtime();

            // Modulamos el maximo y el minimo de la integral para cada proceso

            max=(rank+1)/size;
            min=rank/size;

            // Cálculo de la integral para eses límites, varía en cada proceso

            h=(max-min)/divisiones;

            valor_fijo=(sqrt(1-min*min)+sqrt(1-max*max))/2;
            suma=0;

            for(int i=1;i<divisiones-1;i++){
                valor=min+i*h;
                suma+=(sqrt(1-valor*valor));
            }

            // Resultado de la suma individual de cada proceso

            miParte=h*(valor_fijo+suma);

            // Si el rango del proceso es 0, recibirá los mensajes de todo el mundo, sino, envía al 0
            if(!rank){
                eDefinitivo=miParte;
                for(int i=0;i<size-1;i++){
                    MPI_Recv(&datos_recibidos[i],1,MPI_LONG_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    eDefinitivo+=datos_recibidos[i];
                }

            }else{ 
                MPI_Send(&miParte, 1, MPI_LONG_DOUBLE, 0, 0, MPI_COMM_WORLD);
                
            }

            // Medimos el final del código
            final=MPI_Wtime()-inicio;
            break;

        // Los impares se lo mandan al par inferior
        case 1:
            // Comenzamos a medir el tiempo de la ejecución de cada proceso

            inicio=MPI_Wtime();

            // Modulamos el maximo y el minimo de la integral para cada proceso
            max=(rank+1)/size;
            min=rank/size;

            // Cálculo de la integral para eses límites, varía en cada proceso

            h=(max-min)/divisiones;

            valor_fijo=(sqrt(1-min*min)+(sqrt(1-max*max)))/2;
            suma=0;

            for(int i=1;i<divisiones-1;i++){
                valor=min+i*h;
                suma+=(sqrt(1-valor*valor));
            }

            // Resultado de la suma individual de cada proceso

            miParte=h*(valor_fijo+suma);

            // Si el rango del proceso es impar, se lo mandará al par anterior, el cual, se lo mandará al 0.
            if(!rank){
                int numProc=ceil(size/2);
                eDefinitivo=miParte;
                for(int i=0;i<numProc;i++){
                    MPI_Recv(&datos_recibidos[i],1,MPI_LONG_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    eDefinitivo+=datos_recibidos[i];
                }

            }else if(rank%2==1){ 
                MPI_Send(&miParte, 1, MPI_LONG_DOUBLE, rank-1, 0, MPI_COMM_WORLD);
            }else{
                if(rank!=size-1){
                    MPI_Recv(&datos_recibidos[0],1,MPI_LONG_DOUBLE,rank+1,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    miParte+=datos_recibidos[0];
                }

                MPI_Send(&miParte, 1, MPI_LONG_DOUBLE, 0, 0, MPI_COMM_WORLD);
            }

            // Medimos el final del código
            final=MPI_Wtime()-inicio;
            break;

        case 2:
            // Comenzamos a medir el tiempo de la ejecución de cada proceso

            inicio=MPI_Wtime();

            // Modulamos el maximo y el minimo de la integral para cada proceso

            max=(rank+1)/size;
            min=rank/size;

            // Cálculo de la integral para eses límites, varía en cada proceso

            h=(max-min)/divisiones;

            valor_fijo=(sqrt(1-min*min)+(sqrt(1-max*max)))/2;
            suma=0;

            for(int i=1;i<divisiones-1;i++){
                valor=min+i*h;
                suma+=(sqrt(1-valor*valor));
            }

            // Resultado de la suma individual de cada proceso

            miParte=h*(valor_fijo+suma);

            // Cada grupo de cuatro procesos se comunican entre ellos para que el primero 
            // de ellos le mande la suma de todos al 0 y este sume todos
            int grupo=rank/4;          // grupo de 4
            int lider=grupo*4;         // primero del grupo

            if(rank!=lider){
                MPI_Send(&miParte, 1, MPI_LONG_DOUBLE, lider, 0, MPI_COMM_WORLD);
            }
            else{
                for(int i=1; i<4&&(lider+i)<size; i++) {
                    long double temp;
                    MPI_Recv(&temp, 1, MPI_LONG_DOUBLE, lider+i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    miParte+=temp;
                }

                // Todos los líderes (excepto 0) mandan al 0
                if (rank!=0)
                    MPI_Send(&miParte, 1, MPI_LONG_DOUBLE, 0, 0, MPI_COMM_WORLD);
                else {
                    // El 0 recibe de los demás líderes
                    for (int i=1;i<=size/4; i++) {
                        int r=i*4;
                        if (r<size) {
                            long double temp;
                            MPI_Recv(&temp, 1, MPI_LONG_DOUBLE, r, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            miParte += temp;
                        }
                    }
                    eDefinitivo = miParte;
                }
            }

            // Medimos el final del código
            final=MPI_Wtime()-inicio;
            break;

        default:
            if(rank==0){
                printf("\nDel 0 al 2\n");
            }
            break;
    }

    error=pi-(4*eDefinitivo);

    //Espacio para el print en el archivo
    //Se almacenan los datos de cada nodo: np,yo,Version,Generar,Tiempo
    FILE *pif;
    pif = fopen("Datos_nodos_trapecios.csv", "a");
    fprintf(pif, "%d,%d,%d,%d,%.15lf\n", size, rank, metodo, divisiones, final-inicio);

    //Se almacenan los datos globales: np,Version,Generar,Tiempo,e,Error
    if(rank == 0){

        pif = fopen("Datos_nodo0_trapecios.csv", "a");
        fprintf(pif, "%d,%d,%d,%.15lf,%.15Lf,%.15Lf\n", size, metodo, divisiones, final-inicio, 4*eDefinitivo, fabsl(error));
    }

    MPI_Finalize();
    fclose(pif);
    return 0;
}
