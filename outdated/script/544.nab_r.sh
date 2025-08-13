cd /home/xyc/spec2017/benchspec/CPU/544.nab_r/run/run_base_refrate_mytest-m64.0000
setsid ./nab_r_base.mytest-m64 1am0 1122214447 122 > 1am0.out 2>> 1am0.err &
pid_a=$!
sleep 10
pkill -TERM -g $pid_a
