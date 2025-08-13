import os
import re
import csv
import sys

if len(sys.argv) < 3:
    print(f"Usage: python {sys.argv[0]} <dir_path> <partID1> [partID2 ...]")
    sys.exit(1)

input_dir = sys.argv[1]
target_partids = set(int(pid) for pid in sys.argv[2:])
output_csv = "hit_num_by_partid_channel.csv"

results = []

# 这两个正则分别匹配两行
partid_channel_re = re.compile(r"Packet from partID (\d+) on channel (\S+)")
hit_num_re = re.compile(r"now read hit (\d+) read num (\d+) write hit (\d+) write num (\d+)")

for filename in os.listdir(input_dir):
    if not filename.endswith(".out"):
        continue

    file_path = os.path.join(input_dir, filename)
    data = {}  # key=(partID, channel), value=(read_hit, read_num, write_hit, write_num)

    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        prev_partid = None
        prev_channel = None

        for line in f:
            line = line.strip()
            # 判断是否是partID + channel行
            m1 = partid_channel_re.match(line)
            if m1:
                prev_partid = int(m1.group(1))
                prev_channel = m1.group(2)
                continue

            # 判断是否是hit/num行
            m2 = hit_num_re.match(line)
            if m2 and prev_partid is not None and prev_channel is not None:
                if prev_partid in target_partids:
                    read_hit = int(m2.group(1))
                    read_num = int(m2.group(2))
                    write_hit = int(m2.group(3))
                    write_num = int(m2.group(4))
                    # 每次覆盖，最终保存最后出现的结果
                    data[(prev_partid, prev_channel)] = (read_hit, read_num, write_hit, write_num)
                # 处理完后清空缓存，避免错配下一条
                prev_partid = None
                prev_channel = None

    # 输出结果，保证所有 target_partids 都有数据（无数据则N/A）
    for pid in target_partids:
        found = False
        for (p, ch), vals in data.items():
            if p == pid:
                found = True
                rh, rn, wh, wn = vals
                results.append([filename, pid, ch, rh, rn, wh, wn])
        if not found:
            results.append([filename, pid, "N/A", "N/A", "N/A", "N/A", "N/A"])

with open(output_csv, "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["Filename", "PartID", "Channel", "ReadHit", "ReadNum", "WriteHit", "WriteNum"])
    writer.writerows(results)

print(f"提取完成，结果已保存到 {output_csv}")
