import os
import re

sh_dir = "./"  # 替换为你的 .sh 文件目录

BE_NAMES = []
BE_BINS = []
BE_OPTS = []
BE_IDS = []

for fname in sorted(os.listdir(sh_dir)):
    if not fname.endswith(".sh"):
        continue

    be_name = None
    be_bin = None
    be_opt = None
    be_id = None

    with open(os.path.join(sh_dir, fname)) as f:
        lines = f.readlines()

    for line in lines:
        if line.startswith("cd "):
            # 提取编号和名字
            match = re.search(r"/CPU/(\d+)\.(\w+)_r/", line)
            if match:
                be_id = match.group(1)              # 编号，比如 "500"
                be_name = match.group(2)            # 名称，比如 "perlbench"
        elif line.strip().startswith("./") and "_r_base.mytest-m64" in line:
            # 提取 BE_BIN 和 BE_OPTS
            parts = line.strip().split(" ", 1)
            be_bin = parts[0]                      # ./perlbench_r_base.mytest-m64
            be_opt = parts[1] if len(parts) > 1 else ""

    if be_name and be_bin and be_opt and be_id:
        BE_NAMES.append(be_name)
        BE_BINS.append(be_bin)
        BE_OPTS.append(be_opt)
        BE_IDS.append(be_id)

# 输出四个数组
print("BE_IDS = [")
for id in BE_IDS:
    print(f"    \"{id}\",")
print("]\n")

print("BE_NAMES = [")
for name in BE_NAMES:
    print(f"    \"{name}\",")
print("]\n")

print("BE_BINS = [")
for bin_ in BE_BINS:
    print(f"    \"{bin_}\",")
print("]\n")

print("BE_OPTS = [")
for opt in BE_OPTS:
    print(f"    \"{opt}\",")
print("]")
