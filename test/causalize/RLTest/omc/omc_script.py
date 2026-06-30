################################################################################
# Script that executes omc -s -d=execstat filename.mo 10 times and calculates
# the average times of the "matching and sort" and "tearingSystem" phases. It
# repeats these for different values of N = 100, 1000 and 10000, for each
# model: TestRL1.mo, TestRL2.mo and TestRL3.mo.
################################################################################

import sys
import subprocess
import re
import os
from collections import defaultdict

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

    resultados_totales = {}

    for n in valores_n:
        print(f"╔══════════════════════════════════════════════════════════╗")
        print(f"║ Evaluating with N = {n:<36} ║")
        print(f"╚══════════════════════════════════════════════════════════╝")
        
        modified_content = re.sub(r'(\bN\s*=\s*)\d+', lambda match: f"{match.group(1)}{n}", original_content)
        
        temp_filename = f"temp_benchmark_{filename}"
        with open(temp_filename, 'w', encoding='utf-8') as f:
            f.write(modified_content)

        command = ["omc", "-s", "-d=execstat", temp_filename]
        pattern = re.compile(r"Notification:\s*Performance\s*of\s*(.+?):\s*time\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)/")
        fases_tiempos = defaultdict(list)

        resultados_totales[n] = {"matching and sorting": None, "tearingSystem": None}
        error_en_este_n = False

        for i in range(1, runs + 1):
            print(f"Run {i}/{runs}...", end="\r")
            try:
                result = subprocess.run(command, capture_output=True, text=True, check=True)
                output = result.stdout + result.stderr
                
                matches = pattern.findall(output)
                for fase, tiempo_str in matches:
                    if "tearingSystem (simulation)" in fase:
                        fases_tiempos["tearingSystem"].append(float(tiempo_str))
                    elif "matching and sorting" in fase and "initialization" not in fase:
                        fases_tiempos["matching and sorting"].append(float(tiempo_str))
                    
            except subprocess.CalledProcessError as e:
                print(f"\nError: OMC with N={n}, run {i}:")
                print(e.stderr)
                error_en_este_n = True
                break

        if os.path.exists(temp_filename):
            os.remove(temp_filename)

        if error_en_este_n and not fases_tiempos["matching and sorting"] and not fases_tiempos["tearingSystem"]:
            print(f"Skipping N={n}.")
            continue

        for fase in ["matching and sorting", "tearingSystem"]:
            tiempos = fases_tiempos[fase]
            if tiempos:
                resultados_totales[n][fase] = sum(tiempos) / len(tiempos)
        
        print("Finished.                      ")

    print(f"\n\n====================== TIME MEASURES: {filename} ======================")
    print(f"{'N':<10} | {'Matching & Sorting (s)':<25} | {'Tearing System (s)':<25}")
    print("-" * 68)
    
    for n in valores_n:
        fases = resultados_totales.get(n, {"matching and sorting": None, "tearingSystem": None})
        m_time = fases["matching and sorting"]
        t_time = fases["tearingSystem"]
        
        m_str = f"{m_time:.4e}" if m_time and m_time < 0.001 else (f"{m_time:.6f}" if m_time else "N/A")
        t_str = f"{t_time:.4e}" if t_time and t_time < 0.001 else (f"{t_time:.6f}" if t_time else "N/A")
        
        print(f"{n:<10} | {m_str:<25} | {t_str:<25}")
    print("=======================================================================")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: number of runs required.")
        print("Usage: python omc_script.py runs")
        sys.exit(1)

    input_runs = int(sys.argv[1])
    
    lista_n = [100, 1000, 10000]
    
    model_list = ["TestRL1.mo", "TestRL2.mo", "TestRL3.mo"]
    for model in model_list:
        run_omc_benchmarks(model, valores_n=lista_n, runs=input_runs)
        print("")
