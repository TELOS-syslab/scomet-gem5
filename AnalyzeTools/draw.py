import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import sys

palette = [ '#ffe494', '#18becf', '#c8e63c', '#004600']

DataFile = sys.argv[1]
OutFile = sys.argv[2]
df = pd.read_csv(DataFile)

categories = ['moses','silo','masstree','img-dnn','xapian']

MBA = df['MBA']
Pivot = df['Pivot']
MPAM = df['MPAM']

bar_width = 0.2

bar_positions1 = np.arange(len(categories))
bar_positions2 = bar_positions1 + bar_width
bar_positions3 = bar_positions2 + bar_width

plt.figure(figsize=(10,4))

plt.ylim(20,100)
plt.yticks([20,40,60,80,100], fontsize = 24)
plt.ylabel('BE Throughput (%)', fontsize = 24)

plt.xticks((bar_positions1+bar_positions2 + bar_positions3) / 3,
           categories, fontsize = 24)

plt.axhline(y=40, color='black', linestyle='--',
            alpha=0.4, linewidth=1, zorder = 0)
plt.axhline(y=60, color='black', linestyle='--',
            alpha=0.4, linewidth=1, zorder = 0)
plt.axhline(y=80, color='black', linestyle='--',
            alpha=0.4, linewidth=1, zorder = 0)

# 绘制柱状图
bars1 = plt.bar(bar_positions1, MBA, width=bar_width,
                color = palette[0], edgecolor='black', label='MBA')
bars2 = plt.bar(bar_positions2, MPAM, width=bar_width,
                color = palette[1], edgecolor='black', label='MPAM')
bars3 = plt.bar(bar_positions3, Pivot, width=bar_width,
                color = palette[3], edgecolor='black',label='Pivot')

plt.legend(frameon = False, fontsize=18,
           ncol = 4, bbox_to_anchor=(1, 1.22), loc='upper right')
plt.tight_layout()

plt.savefig(OutFile,format='pdf')
print("Saved figure " + OutFile)