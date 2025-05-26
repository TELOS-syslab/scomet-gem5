//
// Created by 汪小林 on 2022/5/24.
//

#ifndef __RESERVOIR_WXL_RDH_HH__
#define __RESERVOIR_WXL_RDH_HH__


#include <cassert>
#include <vector>
#include "mem/mrcsampler/mrc.hh"
#include "base/output.hh"
using namespace std;

namespace gem5
{
namespace mrcsampler
  {

class ReuseDistanceHistogram{
    public:
    virtual void reset() = 0;

    virtual void addReuse(int rd, uint64_t count) = 0;

    virtual bool buildMRC(MissRatioCurve &mrc) = 0;

    virtual int maxReuseDistance() = 0;

    virtual void print(ostream &os = cout) =0;
};

class VectorRDH :public ReuseDistanceHistogram {
protected:
    vector<uint64_t> histogram;

    const int MAX_RD;
    int maxRD = 0;
public:
    VectorRDH(int MAX_RD) : MAX_RD(MAX_RD) {
        histogram.insert(histogram.begin(), MAX_RD, 0);
    }

    VectorRDH(const vector<uint64_t> &histogram) : MAX_RD(histogram.size()) {
        this->histogram = histogram;
    }

    VectorRDH(const VectorRDH &rdh) : histogram(rdh.histogram),MAX_RD(rdh.MAX_RD),
                                      maxRD(rdh.maxRD) {
    }

    void reset() {
        maxRD = 0;
        std::fill(histogram.begin(), histogram.end(), 0);
    }

    void addReuse(int rd, uint64_t count) {
        if (rd < MAX_RD) {
            histogram[rd] += count;
            maxRD = max(maxRD, rd + 1);
        } else {
            histogram[0] += count;
            maxRD = max(maxRD, MAX_RD);
        }
    }

    bool buildMRC(MissRatioCurve &mrc) {
        uint64_t total = histogram[0];
        for (int i = 1; i < maxRD; i++) {
            total += histogram[i];
        }
        if (total == 0) {
            //printf("rdh failed to build mrc:%d\n",maxRD);
            return false;
        }
        mrc.reset();
        mrc.addMissRatio(1);
        uint64_t misses = total;
        for (int i = 1; i < maxRD; i++) {
            misses -= histogram[i];
            mrc.addMissRatio((double) misses / total);
        }
        return true;
    };

    void merge(const VectorRDH &vrdh) {
        maxRD = max(vrdh.maxRD, maxRD);
        for (int i = 0; i < maxRD; i++) {
            histogram[i] += vrdh.histogram[i];
        }
    }

    void merge(const vector<VectorRDH> &rdhs) {
        for (const VectorRDH &rdh: rdhs) {
            merge(rdh);
        }
    }

    virtual int maxReuseDistance() { return maxRD; };

    virtual void print(ostream &os = cout) {
        os<<"rdh"<<endl;
        for (uint64_t v: histogram) {
            os << v << endl;
        }
    }
};
  }// namespace sampler
} // namespace gem5

#endif //RESERVOIR_WXL_RDH_H
