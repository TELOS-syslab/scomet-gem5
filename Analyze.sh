#!/bin/bash

if [ ! -d "Result" ]; then
    mkdir Result
fi

: > Result/95thlats.txt
: > Result/BE.txt

root_dir="m5out/"

find "$root_dir" -type f -name "lats.bin" | while read file; do
    if [ -f "$file" ]; then
        echo "$file"
        echo "$file" >> Result/95thlats.txt
        python3 AnalyzeTools/parselats.py "$file" >> Result/95thlats.txt
    fi
done

find "$root_dir" -type f -name "stats.txt" | while read file; do
    if [ -f "$file" ]; then
        echo "$file"
        echo "$file" >> Result/BE.txt
        python3 AnalyzeTools/parsestats.py "$file" >> Result/BE.txt
    fi
done

python3 AnalyzeTools/txttocsv.py Result/BE.txt Result/BE.csv BE
python3 AnalyzeTools/txttocsv.py Result/BE.txt Result/NormBE.csv Norm
python3 AnalyzeTools/txttocsv.py Result/95thlats.txt Result/95thlats.csv Lats
python3 AnalyzeTools/draw.py Result/NormBE.csv Result/Figure3.pdf
