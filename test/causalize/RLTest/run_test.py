import subprocess
import os
import sys
import shutil

def run_batch_pipeline(models):
    for model_name in models:
        print(f"\n{'='*20} Processing: {model_name} {'='*20}")
        
        # Paths
        base_name = model_name.replace(".mo", "")
        causalize_bin = "../../../bin/causalize"
        target_mo_dir = "./causalized"
        
        # 1. Execute causalize
        try:
            subprocess.run([causalize_bin, model_name], check=True)
            
            # 2. Move file
            os.makedirs(target_mo_dir, exist_ok=True)
            causalized_file = f"{base_name}_causalized.mo"
            target_path = os.path.join(target_mo_dir, model_name)
            
            if os.path.exists(causalized_file):
                shutil.move(causalized_file, target_path)
            else:
                print(f"Error: {causalized_file} not generated.")
                continue

            # 3. Run simulation
            subprocess.run(
                [sys.executable, "./sim_models.py", "-m", f"./{model_name}"], 
                cwd=target_mo_dir, 
                check=True
            )
            
            # 4. Rename/Move simulation.log
            log_src = os.path.join(target_mo_dir, "simulation.log")
            log_dst = os.path.join(target_mo_dir, f"{base_name}_simulation.log")
            
            if os.path.exists(log_src):
                shutil.move(log_src, log_dst)
                print(f"Log renamed to: {base_name}_simulation.log")
            else:
                print("Warning: simulation.log not found.")            
            
        except subprocess.CalledProcessError as e:
            print(f"Pipeline failed for {model_name}: {e}")

if __name__ == "__main__":
    model_list = ["TestRL1.mo", "TestRL2.mo", "TestRL3.mo"]
    run_batch_pipeline(model_list)
