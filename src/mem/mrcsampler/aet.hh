//
// Created by 汪小林 on 2022/5/31.
//

#ifndef __AET__RESERVOIR_WXL_MRCGENERATOR_HH__
#define __AET__RESERVOIR_WXL_MRCGENERATOR_HH__

#include "mem/mrcsampler/base.hh"
#include "mem/mrcsampler/recorder.hh"
#include "params/AetMrcGenerator.hh"
#include "mem/mrcsampler/rth.hh"

namespace gem5
{
namespace mrcsampler
  {

class AetMrcGenerator : public BaseMrcGenerator {
protected:
    VectorRTH rth;
    Recorder recorder;
public:
    AetMrcGenerator(const AetMrcGeneratorParams &p) :
     BaseMrcGenerator(p), rth(max_reusetime),recorder(max_reusetime,sample_set) {
        reset();
     }
    virtual int trace(uint64_t addr) {
        return recorder.insert(addr);
    }

    virtual bool process() {
        //process to get mrc curve 
        rth.reset();
        recorder.getrth(rth);
        //recorder.clear_rth();
        return true;
    }

    virtual bool mrc(MissRatioCurve &mrc) {
        process();
        return rth.buildMRC(mrc);
    }

    virtual bool reset() {
        recorder.reset();
        rth.reset();
        return true;
    }
    virtual void printMRC(ostream &os = cout){
        VectorMRC vmrc;
        vmrc.reset();
        if(rth.buildMRC(vmrc)){
            os<<"aet ";
            vmrc.print(os);
        }
        //rth.print(*os->stream());
        return ;
    }
    
};


  }// namespace sampler
} // namespace gem5
#endif
