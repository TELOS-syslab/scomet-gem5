#!/bin/bash

./build/ARM/gem5.opt \
--outdir=m5out/MPAMsilo \
configs/example/tailfs.py \
-n 5 \
--caches --l2cache --l3cache \
--cpu-type=ArmV8KvmCPU \
--latency_critical_num=1 \
--test_mode="MPAM" \
--l1d_size="64kB" --l1d_assoc=4 --l1i_size="64kB" --l1i_assoc=4 \
--l2_size="512kB" --l2_assoc=8 \
--l3c_size="7680kB" --l3c_assoc=15 \
--l3b_size="512kB" --l3b_assoc=1 \
--mem-type=DDR4_2400_8x8 --mem-size="16GB" \
--bootloader="/home/Resource/binaries/boot.arm64" \
--kernel="/home/Resource/linux-5.10.137/vmlinux" \
--disk="/home/Resource/testimages/ubuntu-image.img" \
--init="/init.addr.gem5" \
--root-device="/dev/vda2" --machine-type VExpress_GEM5 \
--script="scriptstxt/MPAMsilo.txt" \
--kvm2detail > /dev/null 2>&1 &
pid=$!

sleep 1800

ps -p $pid > /dev/null && kill -INT $pid

