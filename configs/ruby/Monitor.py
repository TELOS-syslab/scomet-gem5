import m5
from m5.objects import *
from m5.objects.Monitor import *
from .PartitionPolicy import *
from m5.objects.MrcGenerator import *
from m5.util.convert import *

def setMonitor(monitor, args):
    gen = HybirdGenerator()
    #gen.setmrcs()
    gen.real = RealMrcGenerator()
    gen.aet = AetMrcGenerator()
    listmrcs=[]
    listmrcs.append(gen.aet)
    listmrcs.append(gen.real)
    gen.mrcs = listmrcs
    gen.sample_set = args.sample_set
    gen.sample_way = args.sample_way
    gen.max_reusetime = args.max_reusetime
    #monitor.mrcgen = gen
    monitor.penalty = [1,1]
    monitor.partition = myHybridPolicy()
    monitor.cacheway=int(toMemorySize(args.l3_size) \
            / args.cacheline_size / args.sample_set)
    monitor.l3_assoc = args.l3_assoc
    monitor.l3_set = int(toMemorySize(args.l3_size)\
            / args.cacheline_size / args.l3_assoc)
    monitor.timeout = 40
