import sys
import os
import numpy as np

def getIPC(file_path):
    ipc_lines = []
    ipc_value = 0

    try:
        with open(file_path, 'r') as file:
            for line in file:
                if 'ipc' in line:
                    ipc_lines.append(line.strip())
                    parts = line.split("                     ")
                    if len(parts) >= 2:
                        #core 0 is not used to run BE Tasks
                        if '0' not in parts[0]:
                            ipc_value += float(parts[1])
        print(ipc_value)
        return ipc_value

    except FileNotFoundError:
        print(f"Error: File {file_path} not found")
    except Exception as e:
        print(f"Error: {e}")


statsFile = sys.argv[1]
#print(sys.argv)
BE_ThroughPut = getIPC(statsFile)
