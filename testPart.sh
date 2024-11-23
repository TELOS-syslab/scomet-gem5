/home/zlr/PartQoStailgem/build/ARM/gem5.opt \
--debug-flags=MemBWCalc \
--outdir=m5out/CachePartitionTestImgdnn \
/home/zlr/PartQoStailgem/configs/example/tailfs.py \
-n 8 \
--caches --l2cache --l3cache \
--cpu-type=ArmV8KvmCPU \
--latency_critical_num=1 \
--l1d_size="64kB" --l1d_assoc=4 --l1i_size="64kB" --l1i_assoc=4 \
--l2_size="512kB" --l2_assoc=8 \
--l3c_size="9MB" --l3c_assoc=9 \
--l3b_size='1MB' --l3b_assoc=1 \
--mem-type=DDR4_2400_8x8 --mem-size="16GB" \
--bootloader="/home/zlr/binaries/boot.arm64" \
--kernel="/home/zlr/linux-5.10.137/vmlinux" \
--disk="/home/zlr/QoSimages/ubuntu-image.img" \
--init="/init.addr.gem5" \
--root-device="/dev/vda2" --machine-type VExpress_GEM5 \
--kvm2detail > bandwidth/TestPartImgdnnbw.txt

#--script="/home/llj/tailgem/script.txt" \

