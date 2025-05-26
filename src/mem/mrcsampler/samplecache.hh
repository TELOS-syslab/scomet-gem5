//
// Created by 汪小林 on 2022/5/24.
//

#ifndef __RESERVOIR_WXL_CACHE_HH__
#define __RESERVOIR_WXL_CACHE_HH__

#include "base/types.hh"
#include <vector>
#include <string.h>

using namespace std;
namespace gem5
{
namespace mrcsampler
  {
class SampleCache {
    //uint64_t *Scache;//[MAX_CACHELINE];
    int cache_way;
    int maxc = 0;
    vector<uint64_t>Scache;
public:
    SampleCache(int size): cache_way(size),Scache(size){
        std::fill(Scache.begin(), Scache.end(), 0);
    }
    ~SampleCache(){
    }

    int insert(uint64_t addr) {
        for (int i = 0; i < cache_way; i++) {
            if (Scache[i] == addr) {
                for (int j = i; j > 0; j--) {
                    Scache[j] = Scache[j - 1];
                }
                Scache[0] = addr;
                maxc = std::max(maxc, i + 1);
                return i + 1;
            }
        }
        for (int j = cache_way - 1; j > 0; j--) {
            Scache[j] = Scache[j - 1];
        }
        maxc = cache_way;
        Scache[0] = addr;
        return 0;
    }

    void reset(){
        std::fill(Scache.begin(), Scache.end(), 0);
    }
};


  }// namespace sampler
} // namespace gem5

#endif