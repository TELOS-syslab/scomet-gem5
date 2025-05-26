//
// Created by 汪小林 on 2022/5/31.
//

#ifndef __REAL__RESERVOIR_WXL_MRCGENERATOR_HH__
#define __REAL__RESERVOIR_WXL_MRCGENERATOR_HH__

#include "mem/mrcsampler/base.hh"
#include "params/RealMrcGenerator.hh"
#include <vector>
#include "mem/mrcsampler/samplecache.hh"
#include "mem/mrcsampler/rdh.hh"
#include "base/output.hh"

using namespace std;

namespace gem5
{
namespace mrcsampler
  {

class RealMrcGenerator : public BaseMrcGenerator {
protected:
    vector<SampleCache> samplecache;
    VectorRDH rdh;
public:
    RealMrcGenerator(const RealMrcGeneratorParams &p) : BaseMrcGenerator(p),
     samplecache(sample_set, SampleCache(sample_way)), rdh(sample_way)//cache_way
    {
    }//TOTAL_SET 

    virtual int trace(uint64_t addr) {
        int index = (addr>>6)&(sample_set-1);
        int rd = samplecache[index].insert(addr);
        rdh.addReuse(rd, 1);
        return rd;
    }

    virtual bool process() {
        return true;
    }

    virtual bool mrc(MissRatioCurve &mrc) {
        //printMRC();
        return rdh.buildMRC(mrc);
    }

    virtual bool reset() {
        for(auto &it: samplecache){
            it.reset();
        }
        rdh.reset();
        return true;
    }

    virtual void printMRC(ostream &os = cout) { 
        //print per million traces
        VectorMRC vmrc;
        vmrc.reset();
        if(rdh.buildMRC(vmrc)){
            os<<"real ";
            vmrc.print(os);
        }
        //rdh.print(*os->stream());
        return ;
    }
};


  }// namespace sampler
} // namespace gem5
#endif