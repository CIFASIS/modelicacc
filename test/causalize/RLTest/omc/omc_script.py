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

def run_omc_benchmarks(filename, valores_n, runs=10):
    try:
        with open("../" + filename, 'r', encoding='utf-8') as f:
            original_content = f.read()
    except FileNotFoundError:
        print(f"Error: file '{filename}' not found")
        return

    print(f"Evaluating OMC compilation for file: {filename}")
    print(f"Values of N: {valores_n}")
    print(f"Number of runs: {runs}\n")

    omc_stages = ["Tearing", "Solve"]
    resultados_totales = {}

    for n in valores_n:
        print(f"╔══════════════════════════════════════════════════════════╗")
        print(f"║ Evaluating with N = {n:<36} ║")
        print(f"╚══════════════════════════════════════════════════════════╝")
        
        modified_content = re.sub(r'(\bN\s*=\s*)\d+', lambda match: f"{match.group(1)}{n}", original_content)
        
        temp_filename = f"temp_benchmark_{filename}"
        with open(temp_filename, 'w', encoding='utf-8') as f:
            f.write(modified_content);

        command = ["omc", "-s", "--newBackend", "-d=dumpBackendClocks", temp_filename]
        #pattern = re.compile(r"^(.+?):\s*([\d.]+)\s*ms", re.MULTILINE)
        pattern = re.compile(r"^\s*([\w\s\d]+?)\.+\s*([\d.e+-]+)", re.MULTILINE)
        fases_tiempos = defaultdict(list)

        resultados_totales[n] = dict.fromkeys(omc_stages)
        error_en_este_n = False

        for i in range(1, runs + 1):
            print(f"Run {i}/{runs}...", end="\r")
            try:
                result = subprocess.run(command, capture_output=True, text=True, check=True)
                output = result.stdout + result.stderr
                
                matches = pattern.findall(output)
                for fase, tiempo_str in matches:
                    fase_clean = fase.strip()
                    if fase_clean in omc_stages:
                        fases_tiempos[fase_clean].append(float(tiempo_str))
                    
            except subprocess.CalledProcessError as e:
                print(f"\nError: OMC with N={n}, run {i}:")
                print(e.stderr)
                error_en_este_n = True
                break

        if os.path.exists(temp_filename):
            os.remove(temp_filename)

        if error_en_este_n and not any(fases_tiempos.values()):
            print(f"Skipping N={n}.")
            continue

        for stage in omc_stages:
            tiempos = fases_tiempos[stage]
            if tiempos:
                resultados_totales[n][stage] = 1e3*sum(tiempos) / len(tiempos)

        print("Finished.                      ")

    print(f"\n\n============================== TIME MEASURES: {filename} ==============================")
    print(f"{'N':<10} | {'Stage Name':<35} | {'Average Time (ms)':<20}")
    print("-" * 73)
    
    for n in valores_n:
        fases = resultados_totales.get(n, {})
        for stage in omc_stages:
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
    
    lista_n = [10**i for i in range(2, 5)]
    
    model_list = ["TestRL1.mo", "TestRL2.mo", "TestRL3.mo"]
    for model in model_list:
        run_omc_benchmarks(model, valores_n=lista_n, runs=input_runs)
        print("")
