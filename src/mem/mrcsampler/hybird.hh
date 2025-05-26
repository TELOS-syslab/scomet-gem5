//
// Created by 汪小林 on 2022/5/31.
//

#ifndef __HYBIRD__RESERVOIR_WXL_MRCGENERATOR_HH__
#define __HYBIRD__RESERVOIR_WXL_MRCGENERATOR_HH__

#include "params/HybirdGenerator.hh"
#include "mem/mrcsampler/real.hh"
#include "mem/mrcsampler/aet.hh"
#include "base/output.hh"

namespace gem5
{
namespace mrcsampler
  {

class HybirdGenerator : public BaseMrcGenerator{
protected:
    vector<BaseMrcGenerator*> mrcgens;
    OutputStream *out; 
public:
    typedef HybirdGeneratorParams Params;
    HybirdGenerator(const Params &p) : BaseMrcGenerator(p),
    mrcgens(p.mrcs) {
        //out = simout.create("trace");
        //(*out->stream())<<hex;
    }
    virtual int trace(uint64_t addr) {
        int ret = 0;
        for(auto it:mrcgens){
            ret |= it->trace(addr);
        }
        return ret;
    }

    virtual bool process(){
        for(auto it:mrcgens){
            it->process();
        }
        return true;
    }

    virtual bool mrc(MissRatioCurve &vmrc) {
        return (mrcgens[0])->mrc(vmrc);
        }

     virtual void printMRC(ostream &os = cout) {
        //return;
        //VectorMRC vmrc;
        for(auto it:mrcgens){
            it->printMRC(os);
            //OutputStream *os = simout.create(it->name()+to_string(count));
            //vmrc.reset();
            //if(it->mrc(vmrc)){
            //    vmrc.print(*os->stream());
            //}
            //simout.close(os);
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


  }// namespace sampler
} // namespace gem5


#endif 