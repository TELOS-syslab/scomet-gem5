//
// Created by 汪小林 on 2022/5/31.
//

#ifndef RESERVOIR_WXL_MRCGENERATOR_H
#define RESERVOIR_WXL_MRCGENERATOR_H

#include <vector>
#include <queue>

#include "mem/mrcsampler/recorder.hh"
#include "mem/mrcsampler/samplecache.h"
#include "mem/mrcsampler/mrc.h"
#include "mem/mrcsampler/rth.h"
#include "base/intmath.hh"
#include "params/MrcGenerator.hh"
#include "params/RealMrcGenerator.hh"
#include "params/AetMrcGenerator.hh"
#include "base/intmath.hh"
#include "sim/sim_object.hh"
#include "base/output.hh"
#include "mem/ruby/structures/CacheMemory.hh"
#include "mem/cache/partition_policies/base.hh"
namespace gem5
{
    //rdh max_rd
    //rth max_rt
    //cache MAX_CACHELINE
    //record TOTAL_SET MAX_REUSETIME SET_MASK
    //MAX_CACHELINE

class MrcGenerator : public SimObject{
protected:
    int sample_set;
    int sample_set_mask;
    int sample_set_mask;
    int sample_set_shift;
    int sample_cache_line;

    int set_total = TOTAL_SET;
    uint64_t set_mask = SET_MASK;
    int set_shift = 6;
    int max_cache_line = MAX_CACHELINE;
    ruby::CacheMemory* supercache;

public:
    typedef MrcGeneratorParams Params;
    MrcGenerator(const Params &p) : SimObject(p) {
        assert(isPowerOf2(p.cache_line));
        set_total = p.cache_size/p.cache_line/p.cache_assoc;
        assert(isPowerOf2(set_total));
        set_shift = floorLog2(p.cache_line);
        set_mask = ((1<<(floorLog2(set_total)))-1)<<set_shift;
    }
    virtual bool trace(unsigned long long addr) { return false; };

    virtual bool process() { return false; };

    virtual bool mrc(MissRatioCurve &mrc) { return false; };

    virtual void printMRC() { 
        VectorMRC vmrc;
        if(mrc(vmrc)){
            vmrc.print(cout);
        }
        return ;
        };

    virtual bool reset() = 0;

};

class RealMrcGenerator : public MrcGenerator {
protected:
    vector<SampleCache> cache;
    VectorRDH rdh;

public:
    RealMrcGenerator(const RealMrcGeneratorParams &p) : MrcGenerator(p),
     cache(set_total, Cache(sample_cache_line)), rdh(MAX_CACHELINE)
    {}

    virtual bool trace(unsigned long long addr) {
        int index = ((addr & set_mask) >> set_shift);
        int rd = cache[index].insert(addr);
        rdh.addReuse(rd, 1);
        return true;
    }

    virtual bool process() {
        return true;
    }

    virtual bool mrc(MissRatioCurve &mrc) {
        return rdh.buildMRC(mrc);
    }

    virtual bool reset() {
        cache = vector<Cache>(set_total, Cache());
        rdh.reset();
        return true;
    }
};

class AetMrcGenerator : public MrcGenerator {
protected:
    Recorder recorder;

    VectorRTH rth;

public:
    AetMrcGenerator(const AetMrcGeneratorParams &p) :
     MrcGenerator(p), rth(CRTH_MAX_RT) {}
    
    //AetMrcGenerator() : rth(CRTH_MAX_RT) {}

    virtual bool trace(unsigned long long addr) {
        recorder.insert(addr);
        return true;
    }

    virtual bool process() {
        for (int rt = 0; rt < recorder.rth.size(); rt++) {
            rth.addReuse(rt, recorder.rth[rt]);
        }
        return true;
    }

    virtual bool mrc(MissRatioCurve &mrc) {
        return rth.buildMRC(mrc);
    }

    virtual bool reset() {
        recorder = Recorder();
        rth.reset();
        return true;
    }
};

class HybirdMrcG : public MrcGenerator{
protected:
partition_policy::Base *m_partitionPolicy_ptr;
    vector<MrcGenerator*> mrcgens;
    int count;
    OutputStream *out; 
public:
    typedef HybirdMrcGParams Params;
    HybirdMrcG(const Params &p) : MrcGenerator(p),
    mrcgens(p.mrcs) {
        count = 0;
        out = simout.create("trace");
        (*out->stream())<<hex;
    }
    virtual bool trace(unsigned long long addr) {
        if(count<100000000){
            (*out->stream())<<"0x"<<addr<<endl;
        }
        count++;
        for(auto it:mrcgens){
            it->trace(addr);
        }
        if(count %1000000==0){
            printMRC();
        }
        return true;
    }

    virtual bool process(){
        for(auto it:mrcgens){
            it->process();
        }
        return true;
    }

    virtual bool mrc(MissRatioCurve &mrc) {return false;}

     virtual void printMRC() { 
        //print per million traces
        VectorMRC vmrc;
        for(auto it:mrcgens){
            OutputStream *os = simout.create(it->name()+to_string(count));
            vmrc.reset();
            if(it->mrc(vmrc)){
                vmrc.print(*os->stream());
            }
            simout.close(os);
        }
        return ;
        };

    virtual bool reset(){
        for(auto it:mrcgens){
            it->reset();
        }
        return true;
    }

};

} // namespace gem5

#endif //RESERVOIR_WXL_MRCGENERATOR_H
