#include <vector>
#include <queue>
#include "mem/mrcsampler/base.hh"
#include "base/intmath.hh"

namespace gem5
{
namespace mrcsampler
  {

BaseMrcGenerator::BaseMrcGenerator(const Params &p) : SimObject(p),
    sample_set(p.sample_set), sample_way(p.sample_way),
    max_reusetime(p.max_reusetime) {
      assert(isPowerOf2(sample_set));
    }

void BaseMrcGenerator::printMRC(ostream &os) { 
        VectorMRC vmrc;
        if(mrc(vmrc)){
            vmrc.print(os);
        }
        return ;
        };

  }// namespace sampler
} // namespace gem5
