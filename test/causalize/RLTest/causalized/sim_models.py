#!/usr/bin/env python3
"""
Tuned simulation pipeline script running the QSS Solver model.
Accepts the model path to 
dynamically extract sizing configuration and update model constants.

Requirements:
* QSS Solver installed with python bindings available.

Run:
python ./sim_models.py --model ./airconds_cont/airconds_cont.mo --partition airconds_cont_100_.json-sbg-4-1213.partition
"""

import argparse
import glob
import os
import sys
import logging
import re
from pathlib import Path
import shutil

# Add the qss_solver module to the Python path
# Adjust this path according to your installation
sys.path.insert(0, '/opt/CIFASIS-CONICET/qss-solver/src/python')

try:
    from qss_solver import (
        file_handlers,
        compile_model, execute_model, run,
        annotations, set_annotations,
        constants, set_constants, parameters, set_parameters,
        config, set_config,
        simulation_log, output_files
    )
except ImportError as e:
    logging.info(f"Error importing qss_solver modules: {e}")
    logging.info("Make sure the qss_solver package is in your Python path")
    sys.exit(1)


def concatenate_proc_logs_from_dir(directory, model_name, output_filename=None):
    """Concatenates files with pattern: {model_name}-{proc_number}.log"""
    directory = os.path.expanduser(directory)
    search_pattern = os.path.join(directory, f"{model_name}-*.log")
    files = glob.glob(search_pattern)
    
    if not files:
        logging.info(f"No log files found for model '{model_name}' in: {directory}")
        return

    def extract_proc_num(filename):
        match = re.search(r'-(\d+)\.log$', filename)
        return int(match.group(1)) if match else -1

    files.sort(key=extract_proc_num)

    if not output_filename:
        output_filename = os.path.join(directory, f"{model_name}-combined.log")
    else:
        if not os.path.isabs(output_filename) and os.sep not in output_filename:
            output_filename = os.path.join(directory, output_filename)

    try:
        with open(output_filename, 'a') as outfile:
            for fname in files:
                if os.path.abspath(fname) == os.path.abspath(output_filename):
                    continue
                logging.info(f"Appending {os.path.basename(fname)}...")
                with open(fname, 'r') as infile:
                    outfile.write(infile.read())
                    outfile.write("\n")
        
        logging.info(f"\nSuccessfully created: {output_filename}")
        return output_filename
    except Exception as e:
        logging.info(f"An error occurred while concatenating logs: {e}")


def setup_environment():
    """Configure required environment variables for QSS Solver."""
    qss_base = Path.home() / "work/qss-solver"
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
    
    return env_vars

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
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    
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

def parse_simulation_data(file_path):
    """
    Reads a simulation log file and returns a dictionary 
    with labels as keys and values as floats.
    """
    data = {}
    
    # Regex pattern: looks for 'Key : Value' format
    # It captures the label before the colon and the number after it.
    pattern = re.compile(r"(.+?)\s*:\s*([\d\.]+)")

    try:
        with open(file_path, 'r') as file:
            for line in file:
                match = pattern.search(line)
                if match:
                    key = match.group(1).strip()
                    value = float(match.group(2))
                    data[key] = value
    except FileNotFoundError:
        logging.info(f"Error: The file at {file_path} was not found.")
        return None
        
    return data

def clean_parallel_log(file_path):
    try:
        with open(file_path, 'r') as f:
            content = f.read()

        cleaned = content.replace("Parallel Simulation Statistics:", "")
        cleaned = re.sub(r'\(\d+\.?\d*\s*%\)', '', cleaned)
        cleaned = cleaned.replace("%", "").replace("MBytes", "")
        
        final_lines = [line.strip() for line in cleaned.splitlines()]
        final_output = "\n".join(final_lines)

        with open(file_path, 'w') as f:
            f.write(final_output)
            
        logging.info(f"Successfully cleaned and updated: {file_path}")
    except FileNotFoundError:
        logging.info(f"Error: The file '{file_path}' was not found.")
    except Exception as e:
        logging.info(f"An error occurred: {e}")


def sim_results(log_file):
    logging.info("\nSimulation results:")
    logging.info(log_file)
    log_data = parse_simulation_data(log_file)
    if log_data:
        for key, value in log_data.items():
            logging.info(f"  {key}: {value}")


def process_model_logs(directory, model_name, proc_func):
    directory = os.path.expanduser(directory)
    search_pattern = os.path.join(directory, f"{model_name}-*.log")
    pattern = re.compile(rf"^{re.escape(model_name)}-(\d+)\.log$")
    
    matched_files = []
    for filepath in glob.glob(search_pattern):
        filename = os.path.basename(filepath)
        if pattern.match(filename):
            matched_files.append(filepath)
            
    matched_files.sort(key=lambda x: int(pattern.match(os.path.basename(x)).group(1)))

    if not matched_files:
        #logging.info(f"No files found matching the pattern in {directory}")
        return

    logging.info(f"Found {len(matched_files)} files. Starting processing...")
    for log_path in matched_files:
        proc_func(log_path)
    logging.info("All files processed successfully.")


def extract_size_from_partition(partition_name):
    """
    Extracts the 'size' value from the partition filename layout:
    model_name_size_.json-method-4-1213.partition
    """
    # Pattern tracks the digits between the model prefix separator and the trailing target items
    partition_p = Path(partition_name).resolve()
    logging.info(str(partition_p.name))
    pattern = r'(\d+)_?\.json-[^-]+-(\d+)-'
    # match = re.search(r'(\d+)_?\.json-', partition_name)
    match = re.search(pattern, str(partition_p))
    if match:
        size = int(match.group(1))
        parts = int(match.group(2))
        return size, parts
    else:
        raise ValueError(
            f"Could not parse size and parts from partition filename layout: '{partition_name}'"
        )
    if match:
        return int(match.group(1))
    else:
        raise ValueError(
            f"Could not parse size variant integer from partition filename layout: '{partition_name}'"
        )


def execute_simulation(model_path):
    """
    Compiles and executes the model dynamically, assigning N size constraints first.
    """
    logging.info(f"\n=== Executing Automated Pipeline for: {model_path} ===")
    
    model_p = Path(model_path).resolve()
    model_file = str(model_p)

    # Step 2: Read current model annotations
    logging.info("\nCurrent model annotations:")
    current_ann = annotations(model_file)
    if "MMO_Output" in current_ann:
        del current_ann["MMO_Output"]
    set_annotations(model_file, current_ann)
    current_ann = annotations(model_file)
    
    if current_ann:
        for key, value in current_ann.items():
            logging.info(f"  {key}: {value}")
    else:
        logging.info("  No annotations found")
    
    # Step 3: Compile the model
    logging.info(f"\nCompiling model: {model_file}")
    if compile_model(model_file):
        logging.info("  Compilation successful")
    else:
        logging.info("  Compilation failed")
        return False
    
    # Step 4: Run the simulation
    logging.info(f"\nRunning simulation: {model_file}")
    if execute_model(model_file):
        logging.info("  Simulation completed successfully")
    else:
        logging.info("  Simulation failed")
        return False
    
    # Step 5: Get simulation results
    model = file_handlers.get_file_name(model_file)
    model_log_path = file_handlers.get_full_path(model, 'MMOC_LOG')    
    sim_results(model_log_path)
    process_model_logs(file_handlers.get_base_path(model_log_path, 'MMOC_LOG'), model, sim_results)    
    return True


def main():
    parser = argparse.ArgumentParser(description="QSS Solver Pipeline Utility")
    parser.add_argument(
        '-m', '--model', 
        type=str, 
        required=True, 
        help="Path layout pointing to target Modelica (.mo) file context"
    )
    args = parser.parse_args()

    logging.info("QSS Solver Python API - Autonomous Simulation Example")
    logging.info("=" * 60)
    
    configure_logging()
    setup_environment()
        
    try:
        execute_simulation(args.model)
    except Exception as e:
        logging.info(f"Error during execution pipeline: {e}")
        logging.error(f"Simulation error trace: {e}", exc_info=True)
    
    logging.info("\n" + "=" * 60)
    logging.info("Pipeline execution cycle completed.")


if __name__ == "__main__":
    main()
