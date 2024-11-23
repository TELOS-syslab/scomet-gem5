from m5.SimObject import *
from m5.params import *
from m5.proxy import *
from m5.objects.MrcGenerator import *
from m5.objects.PartitionPolicies import *

class Monitor(SimObject):
    type = 'Monitor'
    cxx_class = 'gem5::Monitor'
    cxx_header = "mem/Monitor.hh"
    mrcgen = Param.BaseMrcGenerator(NULL,"mrcgenerator")
    partition = Param.BasePartitionPolicy(NULL,"partition policy")
    cacheway = Param.Int(16, "mrc cache way")
    timeout = Param.Int(30, "mrc cache way")
    penalty = VectorParam.Float([1,1],"miss penalty")
    l3_assoc = Param.Int(16, "l3 cache way")
    l3_set = Param.Int( "l3 cache set")
    #    self.cacheway=int(toMemorySize(args.l3_size) \
    #         / args.cache_line_size / self.mrcgen.sample_set)

    #def __init__(self,args) -> None:
    #    super().__init__()
        #self.cache_assoc = args.l3_assoc
        #self.cache_size = args.l3_size