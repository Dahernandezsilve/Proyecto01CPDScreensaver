#!/bin/bash

# Configuración
NUM_ITERATIONS=10
EXECUTION_TIME=15
OUTPUT_FILE="results.txt"

# Función para generar un número aleatorio en un rango
function random_in_range() {
    local min=$1
    local max=$2
    echo $((RANDOM % (max - min + 1) + min))
}

# Función para ejecutar un programa y medir el tiempo de ejecución
function run_program() {
    local program=$1
    local max_gifs=$2
    local gliders=$3
    local guns=$4
    local small_gliders=$5

    local start_time=$(date +%s%N)
    
    # Ejecutar el programa con un tiempo máximo de 15 segundos
    timeout ${EXECUTION_TIME} ./$program $max_gifs $gliders $guns $small_gliders > temp_output.txt
    local end_time=$(date +%s%N)
    
    local execution_time=$(grep "Total Execution Time" temp_output.txt | awk '{print $4}')
    if [ -z "$execution_time" ]; then
        execution_time=$((($end_time - $start_time) / 1000000))
    fi
    echo $execution_time
    rm temp_output.txt
}

# Limpiar el archivo de resultados
echo "Iteration,Max GIFs,Gliders,Guns,Small Gliders,Sequential Time (ms),Parallel Time (ms),Speedup,Efficiency" > $OUTPUT_FILE

for ((i=1; i<=NUM_ITERATIONS; i++)); do
    # Generar parámetros aleatorios
    max_gifs=$(random_in_range 1 50)
    gliders=$(random_in_range 1 30)
    guns=$(random_in_range 1 20)
    small_gliders=$(random_in_range 1 10)

    echo "Iteration $i: Max GIFs=$max_gifs, Gliders=$gliders, Guns=$guns, Small Gliders=$small_gliders"

    # Ejecutar el programa secuencial
    echo "Running sequential program..."
    seq_time=$(run_program "mainSecuencial" $max_gifs $gliders $guns $small_gliders)
    echo "Sequential execution time: $seq_time ms"

    # Ejecutar el programa paralelo
    echo "Running parallel program..."
    par_time=$(run_program "mainParalelo2" $max_gifs $gliders $guns $small_gliders)
    echo "Parallel execution time: $par_time ms"

    # Calcular speedup y eficiencia
    if [ "$par_time" -ne 0 ]; then
        speedup=$(echo "scale=2; $seq_time / $par_time" | bc)
        efficiency=$(echo "scale=2; $speedup / $(nproc)" | bc)
    else
        speedup="N/A"
        efficiency="N/A"
    fi

    # Guardar resultados
    echo "$i,$max_gifs,$gliders,$guns,$small_gliders,$seq_time,$par_time,$speedup,$efficiency" >> $OUTPUT_FILE
done

echo "Results have been saved to $OUTPUT_FILE"
