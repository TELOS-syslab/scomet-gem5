import re
import sys
from collections import defaultdict


def extract_sojourns(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    matches = re.findall(r'sjr:\s*([0-9]+(?:\.[0-9]+)?)', content)
    sojourns = [float(m) if '.' in m else int(m) for m in matches]
    return sojourns

def extract_services(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    matches = re.findall(r'service:\s*([0-9]+(?:\.[0-9]+)?)', content)
    services = [float(m) if '.' in m else int(m) for m in matches]
    return services

def extract_ready_time(file_path):
    pattern = re.compile(
        r'ready time (\d+)'
    )
    results = []
    with open(file_path, 'r') as f:
        for line in f:
            matches = pattern.findall(line)
            for match in matches:
                if int(match) > 1000:
                    results.append(int(match))
    # print(results)
    print(sum(results)/len(results))
    return results
    pattern = re.compile(
        r'(cap|row miss|row hit) channel (\d+) rank (\d+) bank (\d+) row (\d+) partid (\d+) (0x[0-9a-fA-F]+) cap (\d+)'
    )
    results = []
    
    hit = 0
    miss = 0
    cap = 0
    with open(file_path, 'r') as f:
        for line in f:
            matches = pattern.findall(line)
            for match in matches:
                result = {
                    "type": match[0],
                    "channel": int(match[1]),
                    "rank": int(match[2]),
                    "bank": int(match[3]),
                    "row": int(match[4]),
                    "partid": int(match[5]),
                    "address": match[6],
                    "cap": int(match[7])
                }
                results.append(result)
                if result["type"] == "row hit":
                    hit += 1  
                if result["type"] == "row miss":
                    miss += 1
                if result["type"] == "cap":
                    cap += 1
                  
    return hit, miss, cap

def count_read_write_hit_miss(file_path):
    with open(file_path, "r") as f:
        lines = f.readlines()

    current_result = {}
    access_count_result = {}
    id_count_result = {}

    access = 0
    for i, line in enumerate(lines):
        if "Packet from partID" in line and i < len(lines) - 1:
            if "now read hit" in lines[i+1]:
                match_packet = re.search(r'partID (\d+) on channel (\w+)', line)
                match_count = re.search(r'read hit (\d+) read num (\d+) write hit (\d+) write num (\d+)', lines[i+1])
                if match_packet and match_count:
                    partid = int(match_packet.group(1))
                    channel = match_packet.group(2)
                    count = [int(match_count.group(j)) for j in range(1, 5)]
                    # print(partid, channel, count)
                    if (channel, partid) not in access_count_result.keys():
                        access_count_result[(channel, partid)] = [0, 0, 0, 0]
                    if (channel, partid) not in id_count_result.keys():
                        id_count_result[(channel, partid)] = [0, 0, 0, 0]
                    if channel not in current_result.keys():
                        for j in range(4):
                            access_count_result[(channel, partid)][j] += count[j]
                            id_count_result[(channel, partid)][j] += count[j]
                    else:
                        for j in range(4):
                            access_count_result[(channel, partid)][j] += (count[j] - current_result[channel][j])
                            id_count_result[(channel, partid)][j] += (count[j] - current_result[channel][j])
                    current_result[channel] = count
                    if partid == 1:
                        print("access: ", access)
                        access += 1
                        for key, value in access_count_result.items():
                            print(key, value)
                        access_count_result = {}
                else:
                    continue
        if "ID:" in line: 
            id_match = re.search(r'\b\w*ID:(\d+)\b', line)
            if id_match:
                id = int(id_match.group(1))
            else:
                print("failed to get id from ", line)
                exit()
            print("id:", id)
            for key, value in id_count_result.items():
                print(key, value)
            id_count_result = {}


file_path = sys.argv[1]
# print(extract_hit(file_path))
# print(extract_cap(file_path))
# extract_ready_time(file_path)
count_read_write_hit_miss(file_path)
service_result = extract_services(file_path)
sojourn_result = extract_sojourns(file_path)
with open(f"{file_path}_latency.out",mode='w') as latency_f:
    for i in range(len(service_result)):
        latency_f.write(f"{service_result[i]} {sojourn_result[i]}\n")
