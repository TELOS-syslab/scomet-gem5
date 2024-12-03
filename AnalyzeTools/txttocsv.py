import sys
import csv

# 读取并解析 BE.txt 文件
def parse_be_file(filename, mode):
    be_data = {'moses':{},
               'imgdnn':{},
               'silo':{},
               'masstree':{},
               'xapian':{}}
    MaxBE = 0
    with open(filename, 'r') as file:
        lines = file.readlines()

        for i in range(0, len(lines), 2):
            file_path = lines[i].strip()
            value = float(lines[i + 1].strip())
            method = extract_method(file_path)
            bench = extract_bench(file_path)
            if (mode == "Norm" and "BE" in file_path):
                MaxBE = value
            if (bench == None):
                continue
            be_data[bench][method] = value
    return be_data, MaxBE

def extract_bench(file_path):
    if "imgdnn" in file_path:
        return "imgdnn"
    elif "moses" in file_path:
        return "moses"
    elif "silo" in file_path:
        return "silo"
    elif "masstree" in file_path:
        return "masstree"
    elif "xapian" in file_path:
        return "xapian"
    return None


def extract_method(file_path):
    if "MBA" in file_path:
        return "MBA"
    elif "Pivot" in file_path:
        return "Pivot"
    elif "MPAM" in file_path:
        return "MPAM"
    return None

def save_to_csv(be_data, output_filename, mode, maxBE):
    try:
        with open(output_filename, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(['','MBA', 'Pivot', 'MPAM'])
            ky = ['moses', 'silo', 'masstree', 'imgdnn', 'xapian']
            for bench in ky:
                row = [bench]
                if mode == "Norm":
                    row.append(be_data[bench]['MBA'] / maxBE * 100)
                    row.append(be_data[bench]["Pivot"] / maxBE * 100)
                    row.append(be_data[bench]["MPAM"] / maxBE * 100)
                else:
                    row.append(be_data[bench]['MBA'])
                    row.append(be_data[bench]["Pivot"])
                    row.append(be_data[bench]["MPAM"])
                writer.writerow(row)

        print(f"Saved File {output_filename}")
    except Exception as e:
        print(f"Error: {e}")

filename = sys.argv[1]
outputfile = sys.argv[2]
mode = sys.argv[3]

be_data, maxBE = parse_be_file(filename, mode)

save_to_csv(be_data, outputfile, mode, maxBE)
