import m5
from m5.objects import *
from m5.objects.PartitionPolicies import *

class myCASSOC(CASSOC):
    part1 = 16
    part2 = 16

class myCPBM(CPBM):
    part1 = 0x3fff
    part2 = 0xc000

class myMIN_MAX(MIN_MAX):
    enable_min = True
    enable_max = True
    softlimit1 = False
    softlimit2 = False
    softlimit3 = False
    softlimit4 = False
    max1 = 16384*8
    max2 = 16384*8
    min1 = 16384*0
    min2 = 16384*0
    max3 = 16384*8
    max4 = 16384*8
    min3 = 16384*0
    min4 = 16384*0

class myHybridPolicy(HybridPolicy):
    #cassoc = myCASSOC()
    #cpbm = myCPBM()
    min_max = myMIN_MAX()