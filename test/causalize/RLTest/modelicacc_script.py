################################################################################
# Script that executes ../../../bin/causalize 10 times and calculates the
# average times all of the causalization phases, and also the compilation of
# the C generated file. It repeats these for different values of
# N = 100, 1000, 10000, 100000 and 1000000 for each model: TestRL1.mo,
# TestRL2.mo and TestRL3.mo.
################################################################################

import sys
import subprocess
import re
import os
from collections import defaultdict
from pathlib import Path
import sys
import time

# TODO: erase dummy functions and import QSS actual functions
def mmoc_compile_model(filename):
    time.sleep(15 / 1000.0)

def compile_c_model(filename):
    time.sleep(15 / 1000.0)

def run_modelicacc_benchmarks(filename, valores_n, runs=10):
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            original_content = f.read()
    except FileNotFoundError:
        print(f"Error: file '{filename}' not found")
        return

    print(f"Evaluating ModelicaCC compilation for file: {filename}")
    print(f"Values of N: {valores_n}")
    print(f"Number of runs: {runs}\n")

    modelicacc_stages = ["Horizontal sorting SBG builder", "Horizontal sorting"
      , "Algebraic loops SBG builder", "Algebraic loops detection"
      , "Tearing SBG builder", "Tearing"
      , "Vertical sorting SBG builder", "Vertical sorting"
      , "GiNaC solve", "Causalization"]
    qss_stages = ["ModelicaCC compilation", "C compilation"]
    resultados_totales = {}

    for n in valores_n:
        print(f"╔══════════════════════════════════════════════════════════╗")
        print(f"║ Evaluating with N = {n:<36} ║")
        print(f"╚══════════════════════════════════════════════════════════╝")
        
        modified_content = re.sub(r'(\bN\s*=\s*)\d+', lambda match: f"{match.group(1)}{n}", original_content)
        
        temp_filename = f"temp_benchmark_{filename}"
        with open(temp_filename, 'w', encoding='utf-8') as f:
            f.write(modified_content);

        command = ["../../../bin/causalize", temp_filename]
        pattern = re.compile(r"^(.+?):\s*([\d.]+)\s*ms", re.MULTILINE)
        fases_tiempos = defaultdict(list)

        resultados_totales[n] = dict.fromkeys(modelicacc_stages) | dict.fromkeys(qss_stages)
        error_en_este_n = False

        for i in range(1, runs + 1):
            print(f"Run {i}/{runs}...", end="\r")
            # ModelicaCC causalization
            try:
                result = subprocess.run(command, capture_output=True, text=True, check=True)
                output = result.stdout + result.stderr
                
                matches = pattern.findall(output)
                for fase, tiempo_str in matches:
                    fase_clean = fase.strip()
                    if fase_clean in modelicacc_stages:
                        fases_tiempos[fase_clean].append(float(tiempo_str))
                    
            except subprocess.CalledProcessError as e:
                print(f"\nError: ModelicaCC with N={n}, run {i}:")
                print(e.stderr)
                error_en_este_n = True
                break

            # QSS compilation
            start_time = time.perf_counter()
            mmoc_compile_model(filename)
            end_time = time.perf_counter()
            execution_time = end_time - start_time
            fases_tiempos[qss_stages[0]].append(float(execution_time))

            start_time = time.perf_counter()
            compile_c_model(filename)
            end_time = time.perf_counter()
            execution_time = end_time - start_time
            fases_tiempos[qss_stages[1]].append(float(execution_time))
                    
        if os.path.exists(temp_filename):
            os.remove(temp_filename)

        if error_en_este_n and not any(fases_tiempos.values()):
            print(f"Skipping N={n}.")
            continue

        for stage in modelicacc_stages:
            tiempos = fases_tiempos[stage]
            if tiempos:
                resultados_totales[n][stage] = sum(tiempos) / len(tiempos)

        for stage in qss_stages:
            tiempos = fases_tiempos[stage]
            if tiempos:
                resultados_totales[n][stage] = sum(tiempos) / len(tiempos)
        
        print("Finished.                      ")

    print(f"\n\n============================== TIME MEASURES: {filename} ==============================")
    print(f"{'N':<10} | {'Stage Name':<35} | {'Average Time (ms)':<20}")
    print("-" * 73)
    
    for n in valores_n:
        fases = resultados_totales.get(n, {})
        for stage in modelicacc_stages + qss_stages:
            tiempo = fases.get(stage)
            
            if tiempo is not None:
                t_str = f"{tiempo:.4e}" if tiempo < 0.001 else f"{tiempo:.6f}"
            else:
                t_str = "N/A"
                
            print(f"{n:<10} | {stage:<35} | {t_str:<20}")
        print("-" * 73)
    print("=======================================================================================")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: number of runs required.")
        print("Usage: python omc_script.py runs")
        sys.exit(1)

    input_runs = int(sys.argv[1])
    
    lista_n = [10**i for i in range(2, 7)]
    
    model_list = ["TestRL1.mo", "TestRL2.mo", "TestRL3.mo"]
    for model in model_list:
        run_modelicacc_benchmarks(model, valores_n=lista_n, runs=input_runs)
        print("")

    directorio_actual = Path(".")

    # Iteramos sobre todos los archivos del directorio
    for archivo in directorio_actual.iterdir():
        # Verificamos que sea un archivo y que contenga alguno de los dos textos en el nombre
        if archivo.is_file() and ("_sbg_input" in archivo.name or "_causalized" in archivo.name):
            try:
                archivo.unlink()  # Elimina el archivo
                print(f"Deleted: {archivo.name}")
            except Exception as e:
                print(f"Unable to delete {archivo.name}: {e}")
