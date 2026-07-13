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
import logging
import time

def configure_logging():
    """Force-reset logging configuration."""
    # 1. Clear any existing handlers
    root_logger = logging.getLogger()
    if root_logger.handlers:
        for handler in root_logger.handlers:
            root_logger.removeHandler(handler)
    
    # 2. Set the level
    root_logger.setLevel(logging.INFO)
    
    # 3. Create a fresh Formatter
    # formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    formatter = logging.Formatter('%(message)s')
    
    # 4. File Handler (absolute path to be certain where it lands)
    log_file_path = os.path.join(os.getcwd(), 'simulation.log')
    file_handler = logging.FileHandler(log_file_path, mode='w')
    file_handler.setFormatter(formatter)
    root_logger.addHandler(file_handler)
    
    # 5. Stream Handler (Console)
    stream_handler = logging.StreamHandler(sys.stdout)
    stream_handler.setFormatter(formatter)
    root_logger.addHandler(stream_handler)
    
    # Immediate test
    logging.info(f"Logging initialized. Outputting to: {log_file_path}")


def run_omc_benchmarks(filename, valores_n, runs=10):
    try:
        with open("../" + filename, 'r', encoding='utf-8') as f:
            original_content = f.read()
    except FileNotFoundError:
        logging.info(f"Error: file '{filename}' not found")
        return

    logging.info(f"Evaluating OMC compilation for file: {filename}")
    logging.info(f"Values of N: {valores_n}")
    logging.info(f"Number of runs: {runs}\n")

    #omc_stages = ["Causalize", "Tearing", "Solve", "C File Size"]
    omc_stages = ["sorting global known variables", "sort global known variables",
                  "postOpt tearingSystem (initialization)", "postOpt tearingSystem (simulation)",
                  "postOpt solveSimpleEquations (simulation)", "postOpt solveSimpleEquations (initialization)",
                  "matching and sorting (n=400) (initialization)", "matching and sorting (n=300)", 
                  "preOpt resolveLoops (simulation)", "Total compile time", "Build C model", 
                  "C File Size"]

    #omc_stages = ["Compile", "C File Size"]

    resultados_totales = {}

    for n in valores_n:
        logging.info(f"╔══════════════════════════════════════════════════════════╗")
        logging.info(f"║ Evaluating with N = {n:<36} ║")
        logging.info(f"╚══════════════════════════════════════════════════════════╝")
        
        modified_content = re.sub(r'(\bN\s*=\s*)\d+', lambda match: f"{match.group(1)}{n}", original_content)
        
        temp_filename = f"temp_benchmark_{filename}"
        with open(temp_filename, 'w', encoding='utf-8') as f:
            f.write(modified_content);

        #command = ["omc", "-s", "--newBackend", "-d=dumpBackendClocks", temp_filename]
        command = ["omc", "-s", "-d=execstat", temp_filename]

        #pattern = re.compile(r"^\s*([\w\s\d]+?)\.+\s*([\d.e+-]+)", re.MULTILINE)
        pattern = re.compile(r"Performance of (?:[^:]+: )?([^:]+): time ([\d.]+)", re.MULTILINE)
        fases_tiempos = defaultdict(list)

        resultados_totales[n] = dict.fromkeys(omc_stages)
        error_en_este_n = False

        for i in range(1, runs + 1):
            logging.info(f"Run {i}/{runs}...") #, end="\r")
            try:
                start_time = time.perf_counter()
                result = subprocess.run(command, capture_output=True, text=True, check=True)
                end_time = time.perf_counter()
                execution_time = end_time - start_time
                fases_tiempos["Total compile time"].append(float(execution_time))
                output = result.stdout + result.stderr
                lineas = output.splitlines()
                output = "\n".join(lineas[19:])
                
                matches = pattern.findall(output)
                for fase, tiempo_str in matches:
                    fase_clean = fase.strip()
                    if fase_clean in omc_stages:
                        fases_tiempos[fase_clean].append(float(tiempo_str))
                
                #fases_tiempos["Compile"].append(float(execution_time))
                base_path = Path("..")
                model_file = os.path.join(base_path, filename)                
                model_p = Path(model_file).resolve()
                model_file = str(model_p)
                base_name = Path(model_file).stem
                c_file_name = f"{base_name}.c"
                file_size = Path(c_file_name).stat().st_size / 1024
                fases_tiempos["C File Size"].append(float(file_size))
                
                base_name = Path(model_file).stem
                comp_file = Path(".") / base_name
                logging.info(f"\nBuilding C model: {comp_file.resolve()}")
                command = ["rm", "-rf", f"{base_name}"]
                result = subprocess.run(command, capture_output=True, check=True)
                start_time = time.perf_counter()
                command = ["make", "-f", f"{base_name}.makefile"]
                result = subprocess.run(command, check=True)
                end_time = time.perf_counter()
                execution_time = end_time - start_time
                fases_tiempos["Build C model"].append(float(execution_time))

            except subprocess.CalledProcessError as e:
                logging.info(f"\nError: OMC with N={n}, run {i}:")
                logging.info(e.stderr)
                error_en_este_n = True
                break

        if os.path.exists(temp_filename):
            os.remove(temp_filename)

        if error_en_este_n and not any(fases_tiempos.values()):
            logging.info(f"Skipping N={n}.")
            continue

        for stage in omc_stages:
            tiempos = fases_tiempos[stage]
            if tiempos:
                #resultados_totales[n][stage] = 1e3*sum(tiempos) / len(tiempos)
                resultados_totales[n][stage] = sum(tiempos) / len(tiempos)

        logging.info("Finished.                      ")

    logging.info(f"\n\n============================== TIME MEASURES: {filename} ==============================")
    logging.info(f"{'N':<10} | {'Stage Name':50} | {'Average Time (ms)':<20}")
    logging.info("-" * 83)
    
    for n in valores_n:
        fases = resultados_totales.get(n, {})
        for stage in omc_stages:
            tiempo = fases.get(stage)
            
            if tiempo is not None:
                t_str = f"{tiempo:.4e}" if tiempo < 0.001 else f"{tiempo:.6f}"
            else:
                t_str = "N/A"
                
            logging.info(f"{n:<10} | {stage:<50} | {t_str:<20}")
        logging.info("-" * 83)
    logging.info("=======================================================================================")

def clean_folder(folder_path):
    """
    Deletes all files in the folder with extensions: .c, .h, .makefile, .sh, .ini
    """
    target_dir = Path(folder_path)
    
    if not target_dir.is_dir():
        print(f"Error: {folder_path} is not a valid directory.")
        return

    # Define the extensions to remove
    extensions_to_remove = {'.c', '.h', '.makefile', '.sh', '.json', '.xml', '.bin'}

    for file_path in target_dir.iterdir():
        # Check if it's a file and if its suffix is in our set
        if file_path.is_file() and file_path.suffix.lower() in extensions_to_remove:
            try:
                file_path.unlink()
                print(f"Deleted: {file_path.name}")
            except Exception as e:
                print(f"Could not delete {file_path.name}: {e}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        logging.info("Error: number of runs required.")
        logging.info("Usage: python omc_script.py runs")
        sys.exit(1)

    input_runs = int(sys.argv[1])
    
    lista_n = [10**i for i in range(4, 5)]
    
    cur_folder = Path(".")
    clean_folder(cur_folder)

    configure_logging()

    model_list = ["TestRL1.mo", "TestRL2.mo", "TestRL3.mo"]
    for model in model_list:
        run_omc_benchmarks(model, valores_n=lista_n, runs=input_runs)
        logging.info("")
