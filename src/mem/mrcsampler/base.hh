//
// Created by 汪小林 on 2022/5/31.
//

#ifndef __BASE__RESERVOIR_WXL_MRCGENERATOR_HH__
#define __BASE__RESERVOIR_WXL_MRCGENERATOR_HH__

#include "sim/sim_object.hh"
#include "params/BaseMrcGenerator.hh"
#include "mem/mrcsampler/mrc.hh"

namespace gem5
{
namespace mrcsampler
  {


class BaseMrcGenerator : public SimObject{
protected:
////MAX_REUSETIME TOTAL_SET aet
//cache_way //TOTAL_SET 
int sample_set;
int sample_way;
int max_reusetime;
//warn cacheline 64
//set_mask = sample_set -1 ;

public:
    typedef BaseMrcGeneratorParams Params;

    BaseMrcGenerator(const Params &p);

    virtual int trace(uint64_t addr) { return -1; };

    virtual bool process() { return false; };

    virtual bool mrc(MissRatioCurve &mrc) { return false; };

    virtual void printMRC(ostream &os = cout);

    virtual bool reset() = 0;

    virtual int getSet(){return sample_set;};

};


  }// namespace sampler
} // namespace gem5
#endif

