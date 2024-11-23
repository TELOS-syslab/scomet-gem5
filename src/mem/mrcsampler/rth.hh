//
// Created by 汪小林 on 2022/5/24.
//

#ifndef __RESERVOIR_WXL_RTH_HH__
#define __RESERVOIR_WXL_RTH_HH__

#include <cassert>
#include <vector>
#include "mem/mrcsampler/mrc.hh"
#include "mem/mrcsampler/rdh.hh"
#include <cmath>
#include "base/output.hh"

using namespace std;

namespace gem5
{
namespace mrcsampler
  {

class ReuseTimeHistogram {
public:
    virtual void addReuse(int rt, uint64_t count) = 0;

    virtual void completeColdMiss(uint64_t count) = 0;

    void completeTotal(uint64_t count) {
        completeColdMiss(count - calcTotalReuse());
    }

    virtual void completeColdMiss(uint64_t count, int maxReuseTime) = 0;

    void completeTotal(uint64_t count, int maxReuseTime) {
        completeColdMiss(count - calcTotalReuse(), maxReuseTime);
    }

    virtual uint64_t scaleToTotalAccesses(uint64_t n) { return 0; };

    virtual uint64_t calcTotalReuse() = 0;

    virtual uint64_t getColdMiss() = 0;

    virtual bool buildRDH(ReuseDistanceHistogram &rdh) = 0;

    virtual bool buildMRC(MissRatioCurve &mrc) = 0;

    virtual void reset() = 0;

    virtual int type() const { return 0; }
    virtual void print(ostream &os = cout) =0;
};

class VectorRTH :public ReuseTimeHistogram {
protected:
    vector<uint64_t> histogram;
    const int MAX_RT;
    int maxSampledReuseTime = 0;
public:

    virtual void print(ostream &os = cout) {
        os<<"rth"<<endl;
        for (uint64_t v: histogram) {
            os << v << endl;
        }
    }

    VectorRTH(int MAX_RT) : MAX_RT(MAX_RT) {
        histogram.insert(histogram.begin(), MAX_RT, 0);
    }

    VectorRTH(const vector<uint64_t> &array) : MAX_RT(histogram.size()) {
        histogram.resize(MAX_RT);
        for (int i = 0; i < MAX_RT; i++) {
            if (array[i]) {
                maxSampledReuseTime = i + 1;
            }
            histogram[i] = array[i];
        }
    }

    void addReuse(int rt, uint64_t count) {
        if (rt < MAX_RT) {
            histogram[rt] += count;
            maxSampledReuseTime = max(rt + 1, maxSampledReuseTime);
        } else {
            histogram[0] += count;
            maxSampledReuseTime = max(maxSampledReuseTime, MAX_RT);
        }
    }

    void completeColdMiss(uint64_t count) {
        if (count > 0) {
            histogram[0] += count;
        }
    }

    void completeColdMiss(uint64_t count, int maxReuseTime) {
        completeColdMiss(count);
        if (maxSampledReuseTime <= maxReuseTime) {
            maxSampledReuseTime = min(maxReuseTime + 1, MAX_RT);
        }
    }

    virtual uint64_t getColdMiss() {
        return histogram[0];
    }

    uint64_t scaleToTotalAccesses(uint64_t n) {
        uint64_t N = n;
        uint64_t total = calcTotalReuse();
        if ((total < n && total * 33 > n * 32 )|| (n < total && n * 33 > total * 32)) {
            return total;
        }
        vector<uint64_t> array(histogram.size());
        int shift = 0;
        if (total < n) {
            while ((total << 1) < n) {
                shift++;
                total <<= 1;
            }
        }
        while (n > 0) {
            while (total > n) {
                shift--;
                total >>= 1;
            }
            for (int i = 0; i < histogram.size(); i++) {
                if (shift > 0) {
                    array[i] += histogram[i] << shift;
                } else if (shift < 0) {
                    array[i] += histogram[i] >> -shift;
                } else {
                    array[i] += histogram[i];
                }
            }
            n -= total;
        }
        histogram = array;
        total = calcTotalReuse();
        assert((total < N && total * 33 > N * 32 )|| (N < total && N * 33 > total * 32));
        return total;
    }

    uint64_t calcTotalReuse() {
        long total = 0;
        for (long r: histogram) {
            total += r;
        }
        return total;
    }

    bool buildRDH(ReuseDistanceHistogram &rdh);

    bool buildMRC(MissRatioCurve &mrc);

    void reset() {
        std::fill(histogram.begin(), histogram.end(), 0);
        maxSampledReuseTime = 0;
    }

    void merge(const VectorRTH &vrth) {
        maxSampledReuseTime = max(vrth.maxSampledReuseTime, maxSampledReuseTime);
        for (int i = 0; i < maxSampledReuseTime; i++) {
            histogram[i] += vrth.histogram[i];
        }
    }

    void merge(const vector<VectorRTH> &rths) {
        for (const VectorRTH &rth: rths) {
            merge(rth);
        }
    }
};

struct Array : vector<double> {

    Array() {
    }

    Array(const vector<double> &v) : vector<double>(v) {
    }

    Array(int n, double v) : vector<double>(n, v) {
    }

    Array(const Array&b): vector<double>(b){        
    }

    double operator[](int i) const {
        return this->at(i);
    }

    double &operator[](int i) {
        return this->at(i);
    }

    double operator()(double t) const {
        int k = t;
        if (k >= this->size()) {
            return this->back();
        }
        if (k < 0) {
            return this->front();
        }
        if (t == k) {
            return this->at(k);
        }
        double P0, P1;
        if (k + 1 >= this->size()) {
            P1 = this->back();
        } else {
            P1 = this->at(k + 1);
        }
        P0 = this->at(k);
        return P0 - (t - k) * (P0 - P1);
    }

    const Array operator-(const Array &a) const {
        Array b;
        int m = min(size(), a.size());
        for (int i = 0; i < m; i++) {
            b.push_back(max(0.0, at(i) - a[i]));
        }
        for (int i = m; i < size(); i++) {
            b.push_back(at(i));
        }
        return b;
    }

    const Array operator+(const Array &a) const {
        Array b;
        int m = min(size(), a.size());
        for (int i = 0; i < m; i++) {
            b.push_back(at(i) + a[i]);
        }
        for (int i = m; i < size(); i++) {
            b.push_back(at(i));
        }
        for (int i = m; i < a.size(); i++) {
            b.push_back(a[i]);
        }
        return b;
    }

    const Array &operator+=(const Array &a) {
        int m = min(size(), a.size());
        for (int i = 0; i < m; i++) {
            at(i) += a[i];
        }
        for (int i = m; i < a.size(); i++) {
            push_back(a[i]);
        }
        return *this;
    }

    const Array &operator*=(double x) {
        int m = size();
        for (int i = 0; i < m; i++) {
            at(i) *= x;
        }
        return *this;
    }

    const Array &operator=(const vector<double> b) {
        this->resize(b.size());
        for (int i = 0; i < size(); i++) {
            at(i) = b[i];
        }
        return *this;
    }

    const Array &operator=(Array &b) {
        this->resize(b.size());
        for (int i = 0; i < size(); i++) {
            at(i) = b[i];
        }
        return *this;
    }

    const Array &operator=(const Array &b) {
        this->resize(b.size());
        for (int i = 0; i < size(); i++) {
            at(i) = b[i];
        }
        return *this;
    }

    const Array operator*(const Array &a) const {
        Array b;
        int m = min(size(), a.size());
        b.reserve(m);
        for (int i = 0; i < m; i++) {
            b.push_back(at(i) * a[i]);
        }
        return b;
    }

    const Array operator*(const double x) const {
        Array b;
        int m = size();
        b.reserve(m);
        for (int k = 0; k < m; k++) {
            b.push_back(at(k) * x);
        }
        return b;
    }

    double dotProduct(const Array &a) const {
        double sum = 0;
        int m = min(a.size(), size());
        for (int k = 0; k < m; k++) {
            sum += at(k) * a[k];
        }
        return sum;
    }
/*
    Array operator*(const DoubleMatrix &dm) const {
        Array b(dm.M, 0);
        int m = min(dm.M, (int) size());
        for (int k = 0; k < dm.M; k++) {
            double sum = 0;
            for (int j = 0; j < m; j++) {
                sum += at(j) * dm[j][k];
            }
            b[k] = sum;
        }
        return b;
    }
*/
    Array filter(const int from, const int to) const {
        Array b;
        b.insert(b.begin(), to, 0);
        int m = min((int) size(), to);
        for (int i = from; i < m; i++) {
            b[i] = at(i);
        }
        return b;
    }

    const Array &merge(const Array &a, int off) {
        int m = min(size(), a.size());
        int n = a.size();
        if (off < 0) {
            return *this;
        }
        for (int i = off; i < m; i++) {
            at(i) += a[i];
        }
        for (int i = m; i < n; i++) {
            push_back(a[i]);
        }
        return *this;
    }

    const Array &merge(int off, const Array &a) {
        int m = size();
        int n = a.size();
        if (off >= 0) {
            if (n + off <= m) {
                for (int i = 0; i < n; i++) {
                    at(i + off) += a[i];
                }
            } else {
                for (int i = off; i < m; i++) {
                    at(i) += a[i - off];
                }
                for (int i = m - off; i < n; i++) {
                    push_back(a[i]);
                }
            }
        } else if (n + off > 0) {
            if (n + off <= m) {
                for (int i = -off; i < n; i++) {
                    at(i + off) += a[i];
                }
            } else {
                for (int i = 0; i < m; i++) {
                    at(i) += a[i - off];
                }
                for (int i = m - off; i < n; i++) {
                    push_back(a[i]);
                }
            }
        }
        return *this;
    }

    const Array &shift(int off) {
        if (off > 0) {
            insert(begin(), off, 0);
        } else if (off < 0) {
            int end = size() + off;
            for (int i = 0; i < end; i++) {
                at(i) = at(i - off);
            }
            resize(end);
        }
        return *this;
    }
/*
    Array transform(const DoubleMatrix &mm) const {
        int n = mm.M;
        Array b(n, 0);
        int m = min(n, (int) size());
        for (int k = 1; k < n; k++) {
            double sum = 0;
            for (int j = 0; j < m; j++) {
                sum += at(j) * mm[j][k];
            }
            b[k] = sum;
        }
        return b;
    }

    Array transform(const DoubleMatrix &mm, int n) const {
        Array b(n, 0);
        assert(n <= mm.M);
        int m = min(mm.M, (int) size());
        for (int k = 1; k < n; k++) {
            double sum = 0;
            for (int j = 0; j < m; j++) {
                sum += at(j) * mm[j][k];
            }
            b[k] = sum;
        }
        return b;
    }
*/
    Array normalize() const {
        int n = size();
        Array b;
        b.resize(n);
        double total = sum();
        double sum = 0;
        for (int k = 1; k < n; k++) {
            if (total > 0)
                sum += b[k] = at(k) / total;
            else
                sum += b[k] = 0;
        }
        b[0] = 1 - sum;
        return b;
    }

    double sum() const {
        double s = 0;
        for (int i = 0; i < size(); i++) {
            s += at(i);
        }
        return s;
    }

    double sum(int from, int to) const {
        double s = 0;
        int m = min((int) size(), to);
        for (int i = from; i < m; i++) {
            s += at(i);
        }
        return s;
    }
/*
    void print() const {
        for (int i = 0; i < size(); i++) {
            cout << at(i) << " ";
        }
        cout << endl;
    }
    */
};
  }// namespace sampler
} // namespace gem5
#endif //RESERVOIR_WXL_RTH_H
