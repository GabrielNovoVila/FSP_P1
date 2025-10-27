#!/bin/bash

# Valores de puntos a generar por nodo
generar_values=(1000 10000 100000 1000000 2000000 5000000)

# Rango de np
min_np=1
max_np=32

# Número de repeticiones
repeticiones=10

# Bucle principal
for generar in "${generar_values[@]}"; do
    for np in $(seq $min_np $max_np); do
        for version in 0 1 2; do

            # Verificar si la versión es válida para este np
            if [[ $version -eq 1 && $np -lt 4 ]]; then
                continue
            fi
            if [[ $version -eq 2 && $np -lt 8 ]]; then
                continue
            fi

            # Repetir 10 veces
            for ((i=1; i<=repeticiones; i++)); do
                
                # Ejecutar el programa
                mpirun -np "$np" Montecarlo "$generar" "$version"
            done
        done
    done
done

