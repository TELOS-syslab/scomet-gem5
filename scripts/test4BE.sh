#!/bin/bash

./build/ARM/gem5.opt \
--outdir=m5out/test4BE \
configs/example/tailfs.py \
-n 5 \
--caches --l2cache --l3cache \
--cpu-type=ArmV8KvmCPU \
--latency_critical_num=0 \
--l1d_size="64kB" --l1d_assoc=4 --l1i_size="64kB" --l1i_assoc=4 \
--l2_size="512kB" --l2_assoc=8 \
--l3c_size="8MB" --l3_assoc=16 \
--mem-type=DDR4_2400_8x8 --mem-size="16GB" \
--bootloader="/home/Resource/binaries/boot.arm64" \
--kernel="/home/Resource/linux-5.10.137/vmlinux" \
--disk="/home/zlr/testimages/ubuntu-image.img" \
--init="/init.addr.gem5" \
--root-device="/dev/vda2" --machine-type VExpress_GEM5 \
--script="scriptstxt/test4bescript.txt" \
--kvm2detail &
pid=$!

sleep 600

ps -p $pid > /dev/null && kill -INT $pid

