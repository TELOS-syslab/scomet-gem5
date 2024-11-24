./build/ARM/gem5.opt \
--outdir=m5out/testBE \
configs/example/tailfs.py \
-n 8 \
--caches --l2cache --l3cache \
--cpu-type=ArmV8KvmCPU \
--latency_critical_num=0 \
--l1d_size="64kB" --l1d_assoc=4 --l1i_size="64kB" --l1i_assoc=4 \
--l2_size="512kB" --l2_assoc=8 \
--l3_size="8MB" --l3_assoc=16 \
--mem-type=DDR4_2400_8x8 --mem-size="16GB" \
--bootloader="/home/Resource/binaries/boot.arm64" \
--kernel="/home/Resource/linux-5.10.137/vmlinux" \
--disk="/home/Resource/testimages/ubuntu-image.img" \
--init="/init.addr.gem5" \
--root-device="/dev/vda2" --machine-type VExpress_GEM5 \
--script="testbescript.txt" \
--kvm2detail

