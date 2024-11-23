#ifndef __LLC_MONITOR_HH__
#define __LLC_MONITOR_HH__

#include "sim/sim_object.hh"
#include "mem/mrcsampler/base.hh"
#include "mem/cache/partition_policies/base.hh"
#include "params/Monitor.hh"
#include <map>
#include <unordered_set>
#include "sim/eventq.hh"
#include "sim/core.hh"
#include "base/output.hh"

namespace gem5{

class Monitor: public SimObject{
    typedef std::unordered_set<Addr> AddrSet;
    Tick mrcMS  = sim_clock::as_int::ms;
    //Tick mrcMS  = 100000000;//debug
    Tick idle;
     enum{
            NEWID,//stage0 new partid
            WARMUP,//stage1 warmup
            CHECK,//stage2 wait and check convergence
            MRC,//stage3 get mrc
            STABLE,//check if stable
            FLUC,
            TIMEOUT,//stage4 timeout
            TFLUC,
            RECHECK,
            TCHECK,
        };
    class mrcState{
        public:
        int state;
        int hit;
        int miss;
        double total;
        double preRatio;
        double idealRatio;
        Tick time;
        int avail;
        mrcsampler::VectorMRC mrc;
        AddrSet addrset;
        OutputStream *os;
        mrcState(){
            state=NEWID;
            hit = 0;
            miss = 0;
            preRatio=0;
            time=0;
            avail=0;
            total = 0;
            idealRatio = 0;
        }

    };
    //use mrc[cache_size] to determine convergence
    //once timeout, monitor this partid and adjust 
    OutputStream *trace;
    int timeout;
    PARTID traceID;
    mrcsampler::VectorMRC premrc;
    mrcsampler::BaseMrcGenerator* mrcgen;
    PartitionPolicy::Base* partition;
    int cacheway;
    std::vector<double>penalty;
    int l3_assoc,l3_set;
    std::map<PARTID,mrcState> mrcstates;
    EventFunctionWrapper EcheckMR;
    EventFunctionWrapper EbuildMR;
    public:

    Monitor(const MonitorParams &p);

    virtual PartitionPolicy::AbstractCandidates filter(
          PartitionPolicy::AbstractCandidates candidates, const Addr address,
             const PARTID partid);
          //get partid bitmap and return available bitmap
      virtual uint64_t getAvail(
          uint64_t bitmap, const Addr address, const PARTID partid);

      virtual void update(ReplaceableEntry& infoEntry);
      virtual void recycle(ReplaceableEntry& infoEntry);

    int cachehit(PARTID, Addr);
    int cachemiss(PARTID,Addr);

    void checkMR();
    void buildMR();

    void adjust();
    void printmrc();

};

}//gem5


#endif