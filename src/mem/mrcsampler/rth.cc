#include "mem/mrcsampler/rth.hh"

#include <deque>

namespace gem5
{
namespace mrcsampler
  {


bool VectorRTH::buildRDH(ReuseDistanceHistogram &rdh) {
    deque<int64_t> P;
    P.clear();
    P.push_front(histogram[0]);
    for (int rt = maxSampledReuseTime - 1; rt >= 1; rt--) {
        P.push_front(P[0] + histogram[rt]);
    }
    if (P[0] == 0) {
        return false;
    }
    long total = P[0];
    long cold_misses = histogram[0];
    rdh.reset();
    rdh.addReuse(0, cold_misses);
    long delta = 0;
    long nrt = 0, nrd = 0, nr = total - cold_misses;
    for (int i = 1, c = 1; i < maxSampledReuseTime; c++) {
        while (delta < total && i < maxSampledReuseTime) {
            delta += P[i];
            nrt += histogram[i];
            i++;
        }
        delta -= total;
        if (i == maxSampledReuseTime) {
            nrd = nrt;
            nrd = min(nrd, nr);
            rdh.addReuse(c, nrd);
            nr -= nrd;
            break;
        }
        if (delta == 0) {
            nrd = nrt;
            nrd = min(nrd, nr);
            rdh.addReuse(c, nrd);
            nr -= nrd;
            nrt = 0;
        } else {
            long drt = (delta > total / 2) ? histogram[i - 1] : histogram[i - 1] / 2;
            nrd = (nrt - drt);
            nrd = min(nrd, nr);
            rdh.addReuse(c, nrd);
            nr -= nrd;
            nrt = drt;
        }
    }
    return true;
}

bool VectorRTH::buildMRC(MissRatioCurve &mrc) {
    deque<int64_t> P;
    P.clear();
    P.push_front(histogram[0]);
    for (int rt = maxSampledReuseTime - 1; rt >= 1; rt--) {
        P.push_front(P[0] + histogram[rt]);
    }
    if (P[0] == 0) {
        return false;
    }
    double total = P[0];
    mrc.reset();
    mrc.addMissRatio(1);
    double sum = 0;
    for (int i = 1, c = 1; i < maxSampledReuseTime; c++) {
        while (sum < c * total && i < maxSampledReuseTime) {
            sum += P[i];
            i++;
        }
        if (i == MAX_RT) {
            mrc.addMissRatio(P[i - 1] / (double) total);
            break;
        }
        double x = P[i - 1];
        double y = P[i - 2];
        double r = x + ((sum - c * total) / x) * (y - x);
        if (r / total >= MAX_RT) {
            break;
        }
        mrc.addMissRatio(r / total);
    }
    return true;
}

  }// namespace sampler
} // namespace gem5
