docker stop scomet 
docker rm scomet
docker run -i -t --name scomet -d --user root  --privileged -v /home/:/home/:rw forgem5 /bin/bash
docker exec -itd scomet bash -c "cd /home/wjy/Pivot && python3 `which scons` build/X86/gem5.opt -j70"

build/X86/gem5.opt \
configs/example/se.py \
-n=2 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/wjy/Tailbench/tailbench/masstree/mttest_gem5se;/home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/lbm_r_base.mytest-m64' \
--options="-j1 mycsba masstree -s1;3000 /home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/eference.dat 0 0 /home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/100_100_130_ldc.of" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="112MB" --l3c_assoc=14 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=16GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="30%" \
> singlerowbuffer.out 

build/X86/gem5.opt \
configs/example/se.py \
-n=2 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/wjy/Tailbench/tailbench/masstree/mttest_gem5se;/home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/lbm_r_base.mytest-m64' \
--options="-j1 mycsba masstree -s1;3000 /home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/eference.dat 0 0 /home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/100_100_130_ldc.of" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="112MB" --l3c_assoc=14 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=16GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="30%" \
--dualRowBuffer
> dualrowbuffer.out 


# build/X86/gem5.opt configs/example/se.py -n=2 --env=/home/llj/Pivot/mastreeenv.txt '--cmd=/home/llj/Tailbench/tailbench/masstree/mttest_gem5se;/home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/lbm_r_base.mytest-m64' '--options=-j1 mycsba masstree -s1;3000 /home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/eference.dat 0 0 /home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/100_100_130_ldc.of' --cpu-type=X86O3CPU --caches --l2cache --l3cache --l1d_size=64kB --l1d_assoc=4 --l1i_size=64kB --l1i_assoc=4 --l2_size=512kB --l2_assoc=8 --l3_size=4MB --mem-type=DDR5_6400_4x8 --mem-size=16GB --dualRowBuffer