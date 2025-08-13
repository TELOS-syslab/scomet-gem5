import re
import os
import sys

input_filename = sys.argv[1]

pattern_latency = re.compile(r'queue:(\d+)\s+service:(\d+)\s+sjr:(\d+)')

output_lines = []

with open(input_filename, 'r') as f:
    for line in f:
        match = pattern.search(line)
        if match:
            queue, service, sjr = match.groups()
            output_lines.append(f"{queue} {service} {sjr}")

output_filename = input_filename.replace('.out', '_latency.out')

with open(output_filename, 'w') as out_f:
    out_f.write('\n'.join(output_lines))
