# docker stop scomet 
# docker rm scomet
# docker run -i -t --name scomet -d --user root  --privileged -v /home/:/home/:rw forgem5 /bin/bash
# docker exec -itd scomet bash -c "cd /home/wjy/Pivot && python3 `which scons` build/X86/gem5.opt -j70"

pkill -f run.sh

build/X86/gem5.opt \
configs/example/se.py \
-n=1 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/llj/dockerspec/benchspec/CPU/523.xalancbmk_r/run/run_base_refrate_mytest-m64.0000/cpuxalan_r_base.mytest-m64' \
--options="-v /home/llj/dockerspec/benchspec/CPU/523.xalancbmk_r/run/run_base_refrate_mytest-m64.0000/t5.xml /home/llj/dockerspec/benchspec/CPU/523.xalancbmk_r/run/run_base_refrate_mytest-m64.0000/xalanc.xsl" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="8MB" --l3c_assoc=1 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=64GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="10%" \
> Results/xalancbmk.out &

sleep 5400
pkill -f gem5.opt

build/X86/gem5.opt \
configs/example/se.py \
-n=1 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/llj/dockerspec/benchspec/CPU/557.xz_r/run/run_base_refrate_mytest-m64.0000/xz_r_base.mytest-m64' \
--options="/home/llj/dockerspec/benchspec/CPU/557.xz_r/run/run_base_refrate_mytest-m64.0000/cld.tar.xz 160 19cf30ae51eddcbefda78dd06014b4b96281456e078ca7c13e1c0c9e6aaea8dff3efb4ad6b0456697718cede6bd5454852652806a657bb56e07d61128434b474 59796407 61004416 6" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="8MB" --l3c_assoc=1 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=64GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="10%" \
> Results/xz.out &

sleep 5400
pkill -f gem5.opt

build/X86/gem5.opt \
configs/example/se.py \
-n=1 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/wjy/Tailbench/tailbench/masstree/mttest_gem5se' \
--options="-j1 mycsba masstree -s1" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="112MB" --l3c_assoc=14 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=64GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="10%" \
> Results/masstree.out &

sleep 5400
pkill -f gem5.opt

build/X86/gem5.opt \
configs/example/se.py \
-n=1 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/llj/dockerspec/benchspec/CPU/520.omnetpp_r/run/run_base_refrate_mytest-m64.0000/omnetpp_r_base.mytest-m64' \
--options="-c General -r 0" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="8MB" --l3c_assoc=1 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=64GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="10%" \
> Results/omnetpp.out &

sleep 5400
pkill -f gem5.opt

build/X86/gem5.opt \
configs/example/se.py \
-n=1 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/llj/dockerspec/benchspec/CPU/544.nab_r/run/run_base_refrate_mytest-m64.0000/nab_r_base.mytest-m64' \
--options="/home/llj/dockerspec/benchspec/CPU/544.nab_r/run/run_base_refrate_mytest-m64.0000/1am0 1122214447 122" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="8MB" --l3c_assoc=1 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=64GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="10%" \
> Results/nab.out &

sleep 5400
pkill -f gem5.opt

build/X86/gem5.opt \
configs/example/se.py \
-n=1 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/llj/dockerspec/benchspec/CPU/541.leela_r/run/run_base_refrate_mytest-m64.0000/leela_r_base.mytest-m64' \
--options="/home/llj/dockerspec/benchspec/CPU/541.leela_r/run/run_base_refrate_mytest-m64.0000/ref.sgf" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="8MB" --l3c_assoc=1 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=64GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="10%" \
> Results/leela.out &

sleep 5400
pkill -f gem5.opt

build/X86/gem5.opt \
configs/example/se.py \
-n=2 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/llj/dockerspec/benchspec/CPU/505.mcf_r/run/run_base_refrate_mytest-m64.0000/mcf_r_base.mytest-m64' \
--options="/home/llj/dockerspec/benchspec/CPU/505.mcf_r/run/run_base_refrate_mytest-m64.0000/inp.in" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="8MB" --l3c_assoc=1 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=64GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="10%" \
> Results/mcf.out &

sleep 5400
pkill -f gem5.opt

build/X86/gem5.opt \
configs/example/se.py \
-n=2 --env='/home/wjy/Pivot/mastreeenv.txt' \
--cmd='/home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/lbm_r_base.mytest-m64' \
--options="3000 /home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/eference.dat 0 0 /home/llj/dockerspec/benchspec/CPU/519.lbm_r/run/run_base_refrate_mytest-m64.0000/100_100_130_ldc.of" \
--cpu-type=X86O3CPU \
--caches --l2cache --l3cache --l1d_size="48kB" --l1d_assoc=12 --l1i_size="32kB" --l1i_assoc=8 --l2_size="2MB"  --l2_assoc=16  \
--l3c_size="8MB" --l3c_assoc=1 --l3b_size="8MB" --l3b_assoc=1 \
--mem-type=DDR5_6400_4x8  --mem-size=64GB \
--latency_critical_num=1 \
--test_mode="MBA" \
--MBACtrl="10%" \
> Results/lbm.out &

sleep 5400
pkill -f gem5.opt