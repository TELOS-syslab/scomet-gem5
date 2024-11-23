/home/zlr/tailgem/build/ARM/gem5.opt \
--debug-flags=MemBWCalc \
/home/zlr/tailgem/configs/example/tailfs.py \
-n 2 \
--caches --l2cache --l3cache \
--cpu-type=ArmV8KvmCPU \
--l1d_size="64kB" --l1d_assoc=4 --l1i_size="64kB" --l1i_assoc=4 \
--l2_size="512kB" --l2_assoc=8 \
--l3_size="2MB" --l2_assoc=16 \
--mem-type=DDR3_1600_8x8 --mem-size="8GB" \
--bootloader="/home/zlr/binaries/boot.arm64" \
--kernel="/home/zlr/linux-5.10.137/vmlinux" \
--disk="/home/zlr/myimages/ubuntu-image.img" \
--init="/init.addr.gem5" \
--root-device="/dev/vda2" --machine-type VExpress_GEM5 \
--kvm2detail

#--script="/home/llj/tailgem/script.txt" \

