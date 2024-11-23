/home/zlr/CriticalPartQoStailgem/build/ARM/gem5.opt \
--debug-flags=CriticalReport \
/home/zlr/CriticalPartQoStailgem/configs/example/tailfs.py \
-n 1 \
--caches --l2cache --l3cache \
--cpu-type=ArmV8KvmCPU \
--latency_critical_num=1 \
--criticaltype1=1 \
--l1d_size="64kB" --l1d_assoc=4 --l1i_size="64kB" --l1i_assoc=4 \
--l2_size="512kB" --l2_assoc=8 \
--l3_size="8MB" --l2_assoc=16 \
--mem-type=DDR4_2400_8x8 --mem-size="16GB" \
--bootloader="/home/zlr/binaries/boot.arm64" \
--kernel="/home/zlr/linux-5.10.137/vmlinux" \
--disk="/home/zlr/testimages/ubuntu-image.img" \
--init="/init.addr.gem5" \
--root-device="/dev/vda2" --machine-type VExpress_GEM5 \
--kvm2detail > TimeReport.txt

#--script="/home/llj/tailgem/script.txt" \

