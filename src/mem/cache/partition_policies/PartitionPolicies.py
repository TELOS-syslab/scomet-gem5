from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

class BasePartitionPolicy(SimObject):
    type = 'BasePartitionPolicy'
    abstract = True
    cxx_class = 'gem5::PartitionPolicy::Base'
    cxx_header = "mem/cache/partition_policies/base.hh"

class CASSOC(BasePartitionPolicy):
    type = 'CASSOC'
    cxx_class = 'gem5::PartitionPolicy::CASSOC'
    cxx_header = "mem/cache/partition_policies/CASSOC.hh"
    part0 = Param.Int64(32,"default partid 0 cassoc")
    part1 = Param.Int64(0,"partid 1 cassoc")
    part2 = Param.Int64(0,"partid 2 cassoc")
    part3 = Param.Int64(0,"partid 3 cassoc")
    part4 = Param.Int64(0,"partid 4 cassoc")
    part5 = Param.Int64(0,"partid 5 cassoc")
    part6 = Param.Int64(0,"partid 6 cassoc")
    part7 = Param.Int64(0,"partid 7 cassoc")
    part8 = Param.Int64(0,"partid 8 cassoc")


class CPBM(BasePartitionPolicy):
    type = 'CPBM'
    cxx_class = 'gem5::PartitionPolicy::CPBM'
    cxx_header = "mem/cache/partition_policies/CPBM.hh"
    part0 = Param.Int64(0xffffff,"default partid 0 cpbm")
    part1 = Param.Int64(0xffffff,"partid 1 cpbm")
    part2 = Param.Int64(0xffffff,"partid 2 cpbm")
    part3 = Param.Int64(0xffffff,"partid 3 cpbm")
    part4 = Param.Int64(0xffffff,"partid 4 cpbm")
    part5 = Param.Int64(0xffffff,"partid 5 cpbm")
    part6 = Param.Int64(0xffffff,"partid 6 cpbm")
    part7 = Param.Int64(0xffffff,"partid 7 cpbm")
    part8 = Param.Int64(0xffffff,"partid 8 cpbm")

class MIN_MAX(BasePartitionPolicy):
    type = 'MIN_MAX'
    cxx_class = 'gem5::PartitionPolicy::MIN_MAX'
    cxx_header = "mem/cache/partition_policies/MIN_MAX.hh"
    enable_min = Param.Bool(False, "enable min control")
    enable_max = Param.Bool(False, "enable max control")

    min0 = Param.Int64(0,"default partid 0 min capacity")
    min1 = Param.Int64(0,"partid 1 min capacity")
    min2 = Param.Int64(0,"partid 2 min capacity")
    min3 = Param.Int64(0,"partid 3 min capacity")
    min4 = Param.Int64(0,"partid 4 min capacity")
    min5 = Param.Int64(0,"partid 5 min capacity")
    min6 = Param.Int64(0,"partid 6 min capacity")
    min7 = Param.Int64(0,"partid 7 min capacity")
    min8 = Param.Int64(0,"partid 8 min capacity")

    max0 = Param.Int64(16384,"default partid 0 max capacity")
    max1 = Param.Int64(16384,"partid 1 max capacity")
    max2 = Param.Int64(0,"partid 2 max capacity")
    max3 = Param.Int64(0,"partid 3 max capacity")
    max4 = Param.Int64(0,"partid 4 max capacity")
    max5 = Param.Int64(0,"partid 5 max capacity")
    max6 = Param.Int64(0,"partid 6 max capacity")
    max7 = Param.Int64(0,"partid 7 max capacity")
    max8 = Param.Int64(0,"partid 8 max capacity")

    softlimit0 = Param.Bool(False, "enable max0 control softlimit")
    softlimit1 = Param.Bool(False, "enable max1 control softlimit")
    softlimit2 = Param.Bool(False, "enable max2 control softlimit")
    softlimit3 = Param.Bool(False, "enable max3 control softlimit")
    softlimit4 = Param.Bool(False, "enable max4 control softlimit")
    softlimit5 = Param.Bool(False, "enable max5 control softlimit")
    softlimit6 = Param.Bool(False, "enable max6 control softlimit")
    softlimit7 = Param.Bool(False, "enable max7 control softlimit")
    softlimit8 = Param.Bool(False, "enable max8 control softlimit")



class HybridPolicy(BasePartitionPolicy):
    type = 'HybridPolicy'
    cxx_class = 'gem5::PartitionPolicy::hybrid_policy'
    cxx_header = "mem/cache/partition_policies/hybrid_policy.hh"
    cassoc = Param.CASSOC(NULL,"cassoc partition policy")
    cpbm = Param.CPBM(NULL,"cpbm partition policy")
    min_max = Param.MIN_MAX(NULL, "min_max partition policy")


