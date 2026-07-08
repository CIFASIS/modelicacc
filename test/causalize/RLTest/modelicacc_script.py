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
import logging
import time
import shutil

# Add the qss_solver module to the Python path
# Adjust this path according to your installation
sys.path.insert(0, '/opt/CIFASIS-CONICET/qss-solver/src/python')

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

try:
    from qss_solver import (
        file_handlers,
        compile_mmoc_model, compile_c_model, compile_model, execute_model, run,
        annotations, set_annotations,
        constants, set_constants, parameters, set_parameters,
        config, set_config,
        simulation_log, output_files
    )
except ImportError as e:
    logging.info(f"Error importing qss_solver modules: {e}")
    logging.info("Make sure the qss_solver package is in your Python path")
    sys.exit(1)

def remove_validation_block(file_path):
    """
    Removes the validation block from the specified file.
    """
    if not os.path.exists(file_path):
        logging.info(f"Error: File not found at {file_path}")
        return

    pattern = re.compile(
        r'# Validate input parameters\n'
        r'if \[ \$# -lt 2 \]; then\n'
        r'   echo "Usage: \$# <FILE> \[OPTIONS\]"\n'
        r'   exit 1\n'
        r'fi\n',
        re.DOTALL
    )

    try:
        with open(file_path, 'r') as f:
            content = f.read()

        # Check if the pattern exists before trying to replace
        if pattern.search(content):
            new_content = pattern.sub('', content)
            
            with open(file_path, 'w') as f:
                f.write(new_content)
            logging.info(f"Successfully removed the validation block from {file_path}")
        else:
            logging.info("Validation block not found in the file.")
            
    except Exception as e:
        logging.info(f"An error occurred: {e}")

def setup_environment():
    """Configure required environment variables for QSS Solver."""
    qss_base = Path.home() / "qss-solver"
    env_vars = {
        'MMOC_BIN': os.path.join(qss_base, 'bin'),
        'MMOC_MODELS': os.path.join(qss_base, 'models'),
        'MMOC_OUTPUT': os.path.join(qss_base, 'output'),
        'MMOC_BUILD': os.path.join(qss_base, 'build')
    }
    
    for var, value in env_vars.items():
        os.environ[var] = value
        logging.info(f"Set {var} = {value}")
    
    for var in ['MMOC_OUTPUT', 'MMOC_BUILD']:
        path = os.environ[var]
        Path(path).mkdir(parents=True, exist_ok=True)
        logging.info(f"Ensured directory exists: {path}")

    mmoc_build_path = os.environ.get('MMOC_BUILD', './')
    remove_validation_block(os.path.join(mmoc_build_path, 'mmoc.sh'))

    return env_vars
    
def move_causalized_files(model_name):
    """
    Moves {modle_name}_causalized.mo to ./causalized/{model_name}.mo
    """
    source_path = Path(".")
    target_folder = source_path / "causalized"
    
    # Create the 'causalized' folder if it doesn't exist
    target_folder.mkdir(exist_ok=True)
    
    # Define source and destination filenames
    base_name = Path(model_name).stem
    old_filename = f"{base_name}_causalized.mo"
    new_filename = f"{base_name}.mo"
    
    source_file = source_path / old_filename
    dest_file = target_folder / new_filename
    
    if source_file.exists():
        try:
            # Moving and renaming in one step
            shutil.move(str(source_file), str(dest_file))
            logging.info(f"Moved and renamed: {old_filename} -> causalized/{new_filename}")
        except Exception as e:
            logging.info(f"Error moving file: {e}")
    else:
        logging.info(f"File {old_filename} not found in {source_path}")

def run_modelicacc_benchmarks(filename, valores_n, runs=10):
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            original_content = f.read()
    except FileNotFoundError:
        logging.info(f"Error: file '{filename}' not found")
        return

    logging.info(f"Evaluating ModelicaCC compilation for file: {filename}")
    logging.info(f"Values of N: {valores_n}")
    logging.info(f"Number of runs: {runs}\n")

    modelicacc_stages = ["Horizontal sorting SBG builder", "Horizontal sorting"
      , "Algebraic loops SBG builder", "Algebraic loops detection"
      , "Tearing SBG builder", "Tearing"
      , "Vertical sorting SBG builder", "Vertical sorting"
      , "GiNaC solve", "Causalization"]
    qss_stages = ["ModelicaCC compilation", "C compilation", "C File Size"]
    resultados_totales = {}

    for n in valores_n:
        logging.info(f"╔══════════════════════════════════════════════════════════╗")
        logging.info(f"║ Evaluating with N = {n:<36} ║")
        logging.info(f"╚══════════════════════════════════════════════════════════╝")
        
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
            logging.info(f"Run {i}/{runs}...") #, end="\r")
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
                logging.info(f"\nError: ModelicaCC with N={n}, run {i}:")
                logging.info(e.stderr)
                error_en_este_n = True
                break
            
            # QSS compilation
            move_causalized_files(filename)
            base_path = Path(".") / "causalized"
            model_file = os.path.join(base_path, filename)
            
            model_p = Path(model_file).resolve()
            model_file = str(model_p)

            cur_folder = Path(".")
            clean_folder(cur_folder)

            logging.info(f"\nCompiling MMOC model: {model_file}")
            start_time = time.perf_counter()
            comp_res = compile_mmoc_model(model_file, 'output')
            end_time = time.perf_counter()
            execution_time = end_time - start_time
            if comp_res:
                fases_tiempos[qss_stages[0]].append(float(execution_time))
                logging.info(f"  MMOC Compilation successful (took {execution_time:.2f} seconds)")
            else:
                logging.info(f"  MMOC Compilation failed (took {execution_time:.2f} seconds)")
                continue

            base_name = Path(model_file).stem
            comp_file = cur_folder / base_name
            logging.info(f"\nBuilding C model: {comp_file.resolve()}")
            command = ["rm", "-rf", f"{base_name}"]
            result = subprocess.run(command, capture_output=True, check=True)
            start_time = time.perf_counter()
            command = ["make", "-f", f"{base_name}.makefile"]
            result = subprocess.run(command, check=True)
            end_time = time.perf_counter()
            execution_time = end_time - start_time
            if comp_res:
                c_file_name = f"{base_name}.c"
                file_size = Path(c_file_name).stat().st_size / 1024
                fases_tiempos[qss_stages[1]].append(float(execution_time))
                fases_tiempos[qss_stages[2]].append(float(file_size))
                logging.info(f"  C Model Compilation successful (took {execution_time:.2f} seconds)")
            else:
                logging.info(f"  C Model Compilation failed (took {execution_time:.2f} seconds)")
                continue
            
        if os.path.exists(temp_filename):
            os.remove(temp_filename)

        if error_en_este_n and not any(fases_tiempos.values()):
            logging.info(f"Skipping N={n}.")
            continue

        for stage in modelicacc_stages:
            tiempos = fases_tiempos[stage]
            if tiempos:
                resultados_totales[n][stage] = sum(tiempos) / len(tiempos)

        for stage in qss_stages:
            tiempos = fases_tiempos[stage]
            if tiempos:
                resultados_totales[n][stage] = sum(tiempos) / len(tiempos)
        
        logging.info("Finished.                      ")

    logging.info(f"\n\n============================== TIME MEASURES: {filename} ==============================")
    logging.info(f"{'N':<10} | {'Stage Name':<35} | {'Average Time (ms)':<20}")
    logging.info("-" * 73)
    
    for n in valores_n:
        fases = resultados_totales.get(n, {})
        for stage in modelicacc_stages + qss_stages:
            tiempo = fases.get(stage)
            
            if tiempo is not None:
                t_str = f"{tiempo:.4e}" if tiempo < 0.001 else f"{tiempo:.6f}"
            else:
                t_str = "N/A"
                
            logging.info(f"{n:<10} | {stage:<35} | {t_str:<20}")
        logging.info("-" * 73)
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
    extensions_to_remove = {'.c', '.h', '.makefile', '.sh', '.ini'}

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
    
    lista_n = [10**i for i in range(2, 7)]
    
    cur_folder = Path(".")


    clean_folder(cur_folder)
    configure_logging()
    setup_environment()

    model_list = ["TestRL1.mo", "TestRL2.mo", "TestRL3.mo"]
    for model in model_list:
        run_modelicacc_benchmarks(model, valores_n=lista_n, runs=input_runs)
        logging.info("")


    # Iteramos sobre todos los archivos del directorio
    for archivo in cur_folder.iterdir():
        # Verificamos que sea un archivo y que contenga alguno de los dos textos en el nombre
        if archivo.is_file() and ("_sbg_input" in archivo.name or "_causalized" in archivo.name):
            try:
                archivo.unlink()  # Elimina el archivo
                logging.info(f"Deleted: {archivo.name}")
            except Exception as e:
                logging.info(f"Unable to delete {archivo.name}: {e}")
