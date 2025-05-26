#include "mem/mrcsampler/mrc.hh"

namespace gem5
{
namespace mrcsampler
  {

void VectorMRC::reset() {
        vmrc.clear();
    };

void VectorMRC::addMissRatio(double mr) {
        vmrc.push_back(mr);
    };

void VectorMRC::trim(double precision) {
        double cmr = vmrc.back();
        for (int i = vmrc.size() - 2; i > 0; i--) {
            double mr = vmrc[i];
            if (mr - cmr < precision) {
                vmrc.pop_back();
            }
        }
    }
double& VectorMRC::operator[](int posi){
        assert(vmrc.size()!=0);
        if(posi>=vmrc.size()){
            posi = vmrc.size()-1;
        }
        return vmrc[posi];
    }

int VectorMRC::size(){
        return vmrc.size();
    }

void 
VectorMRC::assign(VectorMRC mrc,int size){
    vmrc.assign(mrc.vmrc.begin(),mrc.vmrc.begin()+min(size,mrc.size()));
    while(vmrc.size()<size){
        vmrc.push_back(0);
    }
}

  }// namespace sampler
} // namespace gem5
    