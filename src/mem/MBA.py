from ast import Param
from m5.params import *
from m5.proxy import *
from m5.objects.ClockedObject import ClockedObject

class MBA(ClockedObject):
    type = 'MBA'
    cxx_header = "mem/MBA.hh"
    cxx_class = 'gem5::MBA'
    cpu_side = ResponsePort("This port responds to cpu requests")
    mem_side = RequestPort("This port sends requests to memory")

    MBACycles = Param.Unsigned(0, "MBACycles")