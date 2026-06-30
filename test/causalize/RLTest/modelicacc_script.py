################################################################################
# Script that executes ../../../bin/causalize 10 times and calculates the
# average times for the algebraic loops and complete causalization phases. It
# repeats these for different values of N = 100, 1000, 10000, 100000 and 1000000
# for each model: TestRL1.mo, TestRL2.mo and TestRL3.mo.
################################################################################

import sys
import subprocess
import re
import os
from collections import defaultdict
from pathlib import Path

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

        resultados_totales[n] = {"loops builder": None, "loops": None, "causalize": None}
        error_en_este_n = False

        for i in range(1, runs + 1):
            print(f"Run {i}/{runs}...", end="\r")
            try:
                result = subprocess.run(command, capture_output=True, text=True, check=True)
                output = result.stdout + result.stderr
                
                matches = pattern.findall(output)
                for fase, tiempo_str in matches:
                    if "Algebraic loops SBG builder" in fase:
                        fases_tiempos["loops builder"].append(float(tiempo_str))
                    elif "Algebraic loops detection" in fase:
                        fases_tiempos["loops"].append(float(tiempo_str))
                    elif "Causalization" in fase:
                        fases_tiempos["causalize"].append(float(tiempo_str))
                    
            except subprocess.CalledProcessError as e:
                print(f"\nError: ModelicaCC with N={n}, run {i}:")
                print(e.stderr)
                error_en_este_n = True
                break

        if os.path.exists(temp_filename):
            os.remove(temp_filename)

        if error_en_este_n and not fases_tiempos["loops builder"] and not fases_tiempos["loops"] and not fases_tiempos["causalize"]:
            print(f"Skipping N={n}.")
            continue

        for fase in ["loops builder", "loops", "causalize"]:
            tiempos = fases_tiempos[fase]
            if tiempos:
                resultados_totales[n][fase] = sum(tiempos) / len(tiempos)
        
        print("Finished.                      ")

    print(f"\n\n============================== TIME MEASURES: {filename} ==============================")
    print(f"{'N':<10} | {'SBG loops builder (ms)':<15} | {'SBG loops detection (ms)':<15} | {'SBG Causalization (ms)':<15}")
    print("-" * 87)
    
    for n in valores_n:
        fases = resultados_totales.get(n, {"loops builder": None, "loops": None, "causalize": None})
        b_time = fases["loops builder"]
        l_time = fases["loops"]
        c_time = fases["causalize"]
        
        b_str = f"{b_time:.4e}" if b_time and b_time < 0.001 else (f"{b_time:.6f}" if b_time else "N/A")
        l_str = f"{l_time:.4e}" if l_time and l_time < 0.001 else (f"{l_time:.6f}" if l_time else "N/A")
        c_str = f"{c_time:.4e}" if c_time and c_time < 0.001 else (f"{c_time:.6f}" if c_time else "N/A")
        
        print(f"{n:<10} | {b_str:<22} | {l_str:<24} | {c_str:<25}")
    print("=======================================================================================")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: number of runs required.")
        print("Usage: python omc_script.py runs")
        sys.exit(1)

    input_runs = int(sys.argv[1])
    
    lista_n = [100, 1000, 10000, 100000, 1000000]
    
    model_list = ["TestRL1.mo", "TestRL2.mo", "TestRL3.mo"]
    for model in model_list:
        run_modelicacc_benchmarks(model, valores_n=lista_n, runs=input_runs)
        print("")

    directorio_actual = Path(".")

    # Contador para saber cuántos archivos se borraron
    archivos_eliminados = 0
    
    # Iteramos sobre todos los archivos del directorio
    for archivo in directorio_actual.iterdir():
        # Verificamos que sea un archivo y que contenga alguno de los dos textos en el nombre
        if archivo.is_file() and ("_sbg_input" in archivo.name or "_causalized" in archivo.name):
            try:
                archivo.unlink()  # Elimina el archivo
                print(f"Deleted: {archivo.name}")
                archivos_eliminados += 1
            except Exception as e:
                print(f"Unable to delete {archivo.name}: {e}")
