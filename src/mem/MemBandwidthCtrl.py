from ast import Param
from m5.params import *
from m5.proxy import *
from m5.objects.ClockedObject import ClockedObject

class MemBandwidthCtrl(ClockedObject):
    type = 'MemBandwidthCtrl'
    cxx_header = "mem/mem_bandwidth_ctrl.hh"
    cxx_class = 'gem5::MemBandwidthCtrl'
    cpu_side = ResponsePort("This port responds to cpu requests")
    mem_side = RequestPort("This port sends requests to memory")

    QoS_Queue_Len_HH = Param.Unsigned(32, "QoS_queue_Len_HH")
    QoS_Queue_Len_H = Param.Unsigned(64, "QoS_queue_Len_H")
    QoS_Queue_Len_M = Param.Unsigned(64, "QoS_queue_Len_M")
    QoS_Queue_Len_L = Param.Unsigned(96, "QoS_queue_Len_L")
    Write_Threshold = Param.Unsigned(8, "Write_Threshold")
    Time_Threshold = Param.Unsigned(4440000, "Write_Threshold")
    TDM_Len_Max = Param.Unsigned(8, "TD_Len_Max")

    Partition_Mode = Param.Unsigned(1, "Partition_Mode")
    Mem_Bandwidth_Monitor_Window = Param.Unsigned(5000000, "Mem_Bandwidth_Monitor_Window")