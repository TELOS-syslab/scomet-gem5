import subprocess
import time
import sys

IMAGE = "scomet:v0.1"
MOUNT_DIR = "/home/wjy"
BUILD_CMD = "cd /home/wjy/Pivot && python3 /usr/bin/scons build/X86/gem5.opt -j70 && python3 /usr/bin/scons build/X86/gem5.debug -j70"
KILL_CMD = "pkill -f gem5.*"
GEM5_BIN = "/home/wjy/Pivot/build/X86/gem5.opt"
LC_BIN = "/home/wjy/Tailbench/tailbench/masstree/mttest_gem5se"
LC_OPTS = "-j1 mycsba masstree -s1"
SPEC_DIR = "/home/wjy/dockerspec/benchspec/CPU/{}/run/run_base_refrate_mytest-m64.0000"

BE_IDS = ["500", "502", "503", "505", "507", "508", "510", "511", "519", "520", "521", "523", "525", "526", "527", "531", "538", "541", "544", "548", "549", "554", "557"]
BE_NAMES = ["perlbench", "gcc", "bwaves", "mcf", "cactuBSSN", "namd", "parest", "povray", "lbm", "omnetpp", "wrf", "xalancbmk", "x264", "blender", "cam4", "deepsjeng", "imagick", "leela", "nab", "exchange2", "fotonik3d", "roms", "xz"]
BE_BINS = [
    f"{SPEC_DIR.format('500.perlbench_r')}/perlbench_r_base.mytest-m64",
    f"{SPEC_DIR.format('502.gcc_r')}/cpugcc_r_base.mytest-m64",
    f"{SPEC_DIR.format('503.bwaves_r')}/bwaves_r_base.mytest-m64",
    f"{SPEC_DIR.format('505.mcf_r')}/mcf_r_base.mytest-m64",
    f"{SPEC_DIR.format('507.cactuBSSN_r')}/cactusBSSN_r_base.mytest-m64",
    f"{SPEC_DIR.format('508.namd_r')}/namd_r_base.mytest-m64",
    f"{SPEC_DIR.format('510.parest_r')}/parest_r_base.mytest-m64",
    f"{SPEC_DIR.format('511.povray_r')}/povray_r_base.mytest-m64",
    f"{SPEC_DIR.format('519.lbm_r')}/lbm_r_base.mytest-m64",
    f"{SPEC_DIR.format('520.omnetpp_r')}/omnetpp_r_base.mytest-m64",
    f"{SPEC_DIR.format('521.wrf_r')}/wrf_r_base.mytest-m64",
    f"{SPEC_DIR.format('523.xalancbmk_r')}/cpuxalan_r_base.mytest-m64",
    f"{SPEC_DIR.format('525.x264_r')}/x264_r_base.mytest-m64",
    f"{SPEC_DIR.format('526.blender_r')}/blender_r_base.mytest-m64",
    f"{SPEC_DIR.format('527.cam4_r')}/cam4_r_base.mytest-m64",
    f"{SPEC_DIR.format('531.deepsjeng_r')}/deepsjeng_r_base.mytest-m64",
    f"{SPEC_DIR.format('538.imagick_r')}/imagick_r_base.mytest-m64",
    f"{SPEC_DIR.format('541.leela_r')}/leela_r_base.mytest-m64",
    f"{SPEC_DIR.format('544.nab_r')}/nab_r_base.mytest-m64",
    f"{SPEC_DIR.format('548.exchange2_r')}/exchange2_r_base.mytest-m64",
    f"{SPEC_DIR.format('549.fotonik3d_r')}/fotonik3d_r_base.mytest-m64",
    f"{SPEC_DIR.format('554.roms_r')}/roms_r_base.mytest-m64",
    f"{SPEC_DIR.format('557.xz_r')}/xz_r_base.mytest-m64"
]
BE_OPTS = [
    f"-I{SPEC_DIR.format('500.perlbench_r')}/lib {SPEC_DIR.format('500.perlbench_r')}/checkspam.pl 2500 5 25 11 150 1 1 1 1",
    f"{SPEC_DIR.format('502.gcc_r')}/gcc-pp.c -O3 -finline-limit=0 -fif-conversion -fif-conversion2 -o {SPEC_DIR.format('502.gcc_r')}/gcc-pp.opts-O3_-finline-limit_0_-fif-conversion_-fif-conversion2.s",
    f"{SPEC_DIR.format('503.bwaves_r')}/bwaves_1 < {SPEC_DIR.format('503.bwaves_r')}/bwaves_1.in",
    f"{SPEC_DIR.format('505.mcf_r')}/inp.in",
    f"{SPEC_DIR.format('507.cactuBSSN_r')}/spec_ref.par",
    f"--input {SPEC_DIR.format('508.namd_r')}/apoa1.input --output {SPEC_DIR.format('508.namd_r')}/apoa1.ref.output --iterations 65",
    f"{SPEC_DIR.format('510.parest_r')}/ref.prm",
    f"{SPEC_DIR.format('511.povray_r')}/SPEC-benchmark-ref.ini",
    f"3000 {SPEC_DIR.format('519.lbm_r')}/reference.dat 0 0 {SPEC_DIR.format('519.lbm_r')}/100_100_130_ldc.of",
    "-c General -r 0",
    "",
    f"-v {SPEC_DIR.format('523.xalancbmk_r')}/t5.xml {SPEC_DIR.format('523.xalancbmk_r')}/xalanc.xsl",
    f"--pass 1 --stats {SPEC_DIR.format('525.x264_r')}/x264_stats.log --bitrate 1000 --frames 1000 -o {SPEC_DIR.format('525.x264_r')}/BuckBunny_New.264 {SPEC_DIR.format('525.x264_r')}/BuckBunny.yuv 1280x720",
    f"{SPEC_DIR.format('526.blender_r')}/sh3_no_char.blend --render-output sh3_no_char_ --threads 1 -b -F RAWTGA -s 849 -e 849 -a",
    "",
    f"{SPEC_DIR.format('531.deepsjeng_r')}/ref.txt",
    f"-limit disk 0 {SPEC_DIR.format('538.imagick_r')}/refrate_input.tga -edge 41 -resample 181% -emboss 31 -colorspace YUV -mean-shift 19x19+15% -resize 30% {SPEC_DIR.format('538.imagick_r')}/refrate_output.tga",
    f"{SPEC_DIR.format('541.leela_r')}/ref.sgf",
    f"{SPEC_DIR.format('544.nab_r')}/1am0 1122214447 122",
    "6",
    "",
    f"< {SPEC_DIR.format('554.roms_r')}/ocean_benchmark2.in.x",
    f"{SPEC_DIR.format('557.xz_r')}/cld.tar.xz 160 19cf30ae51eddcbefda78dd06014b4b96281456e078ca7c13e1c0c9e6aaea8dff3efb4ad6b0456697718cede6bd5454852652806a657bb56e07d61128434b474 59796407 61004416 6"
]

BE_IDS = ["0"] + BE_IDS
BE_NAMES = ["baseline"] + BE_NAMES
BE_BINS = ["/home/wjy/Pivot/dummy/do_nothing"] + BE_BINS
BE_OPTS = [""] + BE_OPTS

def run_cmd(cmd):
    print(f"[RUN] {cmd}")
    return subprocess.run(cmd, shell=True, check=False)

def cleanup_scomet_containers():
    print("Cleaning up all scomet* containers...")
    result = subprocess.run("docker ps -a --filter name=^scomet --format '{{.ID}}'", shell=True, capture_output=True, text=True)
    containers = result.stdout.strip().splitlines()
    if not containers:
        print("No containers to clean.")
        return
    for cid in containers:
        run_cmd(f"docker stop {cid}")
        run_cmd(f"docker rm {cid}")
    print("Cleanup done.")

def run_container(name):
    run_cmd(f"docker stop {name} 2>/dev/null")
    run_cmd(f"docker rm {name} 2>/dev/null")
    run_cmd(f"docker run -itd --name {name} --user root --privileged -v {MOUNT_DIR}:{MOUNT_DIR}:rw {IMAGE} /bin/bash")
    if name == "scomet":
        run_cmd(f"docker exec -itd {name} bash -c \"{BUILD_CMD}\"")
        while True:
            result1 = subprocess.run(
                f"docker exec {name} test -f /home/wjy/Pivot/build/X86/gem5.opt",
                shell=True
            )
            result2 = subprocess.run(
                f"docker exec {name} test -f /home/wjy/Pivot/build/X86/gem5.debug",
                shell=True
            )
            if result1.returncode == 0 and result2.returncode == 0:
                print("gem5 build complete.")
                break
            print("Waiting for gem5 build...")
            time.sleep(30)

def run_gem5(container, n, rps, cmd_str, opts_str, cha, be_mbw, outfile):
    ENV_PATH = f"/home/wjy/Pivot/env/mastreeenv_{rps}.txt"
    lines = [
        "TBENCH_MAXREQS=3000\n",
        f"TBENCH_WARMUPREQS={rps*5}\n",
        "TBENCH_MINSLEEPNS=10000\n",
        "TBENCH_RANDSEED=0\n",
        f"TBENCH_QPS={rps}\n",
    ]
    with open(ENV_PATH, 'w') as f:
        f.writelines(lines)
    run_cmd(f"mkdir -p /home/wjy/Pivot/checkpoints/{container}")

    if n == 1:
        gem5_cmd = (
            f"cd /home/wjy/Pivot/run && "
            f"{GEM5_BIN} /home/wjy/Pivot/configs/example/se.py "
            f"-n={n} --env='{ENV_PATH}' --cmd='{cmd_str}' --options='{opts_str}' "
            f"--cpu-type=X86O3CPU --caches --l2cache --l3cache "
            f"--l1d_size=48kB --l1d_assoc=12 --l1i_size=32kB --l1i_assoc=8 "
            f"--l2_size=2MB --l2_assoc=16 --l3c_size=120MB --l3c_assoc=15 "
            f"--l3b_size=120MB --l3b_assoc=15 --mem-type=DDR5_6400_4x8 "
            f"--mem-size=16GB --mem-channels={cha} "
            f"--latency_critical_num={n-1} --test_mode=MBA --MBACtrl={be_mbw}% "
            f"--checkpoint-dir=/home/wjy/Pivot/checkpoints/{container}"
            f"--take-checkpoints=100"
            f"> /home/wjy/Pivot/Results/{outfile} 2>&1"
        )
    else:
        gem5_cmd = (
            f"cd /home/wjy/Pivot/run && "
            f"{GEM5_BIN} /home/wjy/Pivot/configs/example/se.py "
            f"-n={n} --env='{ENV_PATH}' --cmd='{cmd_str}' --options='{opts_str}' "
            f"--cpu-type=X86O3CPU --caches --l2cache --l3cache "
            f"--l1d_size=48kB --l1d_assoc=12 --l1i_size=32kB --l1i_assoc=8 "
            f"--l2_size=2MB --l2_assoc=16 --l3c_size=112MB --l3c_assoc=14 "
            f"--l3b_size=8MB --l3b_assoc=1 --mem-type=DDR5_6400_4x8 "
            f"--mem-size=16GB --mem-channels={cha} "
            f"--latency_critical_num={n-1} --test_mode=MBA --MBACtrl={be_mbw}% "
            f"--checkpoint-dir=/home/wjy/Pivot/checkpoints/{container}"
            f"--take-checkpoints=100"
            f"> /home/wjy/Pivot/Results/{outfile} 2>&1"
        )
    run_cmd(f"docker exec -itd {container} bash -c \"{gem5_cmd}\"")

def run_lc_be_gem5(container, n, rps, be_name, be_bin, be_opts, channels, be_mbw=10):
    out_file = f"{n-1}masstree_{rps}rps_1{be_name}_{channels}channels.out"
    bin_list = [LC_BIN] * (n-1) + [be_bin]
    opt_list = [LC_OPTS] * (n-1) + [be_opts]
    cmd_str = ";".join(bin_list)
    opts_str = ";".join(opt_list)

    run_container(container)
    run_gem5(container, n, rps, cmd_str, opts_str, channels, be_mbw, out_file)

def main():
    cleanup_scomet_containers()
    run_container("scomet")
    time.sleep(10)

    if "--debug" in sys.argv:
        GEM5_BIN = "/home/wjy/Pivot/build/X86/gem5.debug --debug-flags=Decoder"

    count = 0
    CHANNELS = [8]
    TASK_NUM = [1, 16]
    RPS = [25]
    # USED_BE = ["baseline", "cactuBSSN", "gcc", "perlbench", "xalancbmk"]
    USED_BE = BE_NAMES
    for i in range(len(BE_NAMES)):
        for ch in CHANNELS:
            for n in TASK_NUM:
                for rps in RPS:
                    be_name = BE_NAMES[i]
                    if be_name not in USED_BE:
                        continue
                    be_bin = BE_BINS[i]
                    be_opts = BE_OPTS[i]
                    container_id = f"scomet{count}"
                    print(f"Launching {container_id}")
                    if TASK_NUM == 1:
                        run_lc_be_gem5(container_id, n, rps, be_name, be_bin, be_opts, ch, 100)
                    else:
                        run_lc_be_gem5(container_id, n, rps, be_name, be_bin, be_opts, ch, 10)
                    count += 1

    time.sleep(86400*3)

    print("Killing gem5 in all containers...")
    for i in range(count):
        container_id = f"scomet{i}"
        run_cmd(f"docker exec -itd {container_id} bash -c \"{KILL_CMD}\"")

if __name__ == "__main__":
    main()