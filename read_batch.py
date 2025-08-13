import os
import subprocess
import sys

input_dir = sys.argv[1]
output_dir = sys.argv[1]

for file_name in os.listdir(input_dir):
    if file_name.endswith(".out") and "latency" not in file_name:
        input_path = os.path.join(input_dir, file_name)
        base_name = os.path.splitext(file_name)[0]
        # output_path = os.path.join(output_dir, f"{base_name}_queue.log")

        print(f"[RUNNING] python3 read.py {input_path}")
        # with open(output_path, "w") as fout:
        #    subprocess.run(["python3", "read.py", input_path], stdout=fout, stderr=subprocess.STDOUT)
        subprocess.run(["python3", "read.py", input_path], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

