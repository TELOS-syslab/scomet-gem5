import os
import pandas as pd
import sys

# 设置目录和读取目标文件
results_dir = sys.argv[1]
latency_files = [f for f in os.listdir(results_dir) if 'latency' in f and os.path.isfile(os.path.join(results_dir, f))]

columns = {}
max_len = 0

# 提取每个文件的最后一列，记录最大长度
for filename in sorted(latency_files):
    filepath = os.path.join(results_dir, filename)
    try:
        df = pd.read_csv(filepath, delim_whitespace=True, header=None)
        if df.empty:
            raise ValueError("文件为空")
        last_col = df.iloc[:, -1]
        columns[filename] = last_col
        max_len = max(max_len, len(last_col))
    except Exception as e:
        print(f"读取文件 {filename} 失败：{e}")
        columns[filename] = pd.Series([], dtype=float)  # 空列也要加入

# 设置最大长度为所有列中最长的，或至少为1（防止所有文件失败）
max_len = max(max_len, 1)

# 对每一列填充 0 使其与最长长度对齐
padded = {}
for name, col in columns.items():
    padded_col = col.tolist() + [0] * (max_len - len(col))
    padded[name] = padded_col

# 合并并输出
result_df = pd.DataFrame(padded)
output_path = os.path.join('./latency_matrix.csv')
result_df.to_csv(output_path, index=False)
print(f"提取完成，共保留 {max_len} 行（不足补 0），已保存为 {output_path}")
