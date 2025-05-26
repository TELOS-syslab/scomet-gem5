from m5.SimObject import *
from m5.params import *
from m5.proxy import *

class BaseMrcGenerator(SimObject):
    type = 'BaseMrcGenerator'
    abstract = True
    cxx_class = 'gem5::mrcsampler::BaseMrcGenerator'
    cxx_header = "mem/mrcsampler/base.hh"
    #cache_size = Param.MemorySize("capacity in bytes")
    #cache_line = Param.Int(Parent.cache_line_size, "block size in bytes")
    #cache_assoc = Param.Int("")
    sample_set = Param.Int(2048, "sample set")
    sample_way = Param.Int(256, "sample way")
    max_reusetime = Param.Int(1024, "max_reusetime")

    #@cxxMethod()
    #def printMRC(self):
    #    pass

class RealMrcGenerator(BaseMrcGenerator):
    type = 'RealMrcGenerator'
    cxx_class = 'gem5::mrcsampler::RealMrcGenerator'
    cxx_header = "mem/mrcsampler/real.hh"

        
class AetMrcGenerator(BaseMrcGenerator):
    type = 'AetMrcGenerator'
    cxx_class = 'gem5::mrcsampler::AetMrcGenerator'
    cxx_header = "mem/mrcsampler/aet.hh"


class HybirdGenerator(BaseMrcGenerator):
    type = 'HybirdGenerator'
    cxx_class = 'gem5::mrcsampler::HybirdGenerator'
    cxx_header = "mem/mrcsampler/hybird.hh"
    mrcs = VectorParam.BaseMrcGenerator([],"a set of MRCG")
    def setmrcs(self):
        self.real = RealMrcGenerator()
        self.aet = AetMrcGenerator()
        listmrcs=[]
        listmrcs.append(self.aet)
        listmrcs.append(self.real)
        self.mrcs = listmrcs
        


