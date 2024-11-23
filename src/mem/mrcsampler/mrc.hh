#ifndef __RESERVOIR_WXL_MRC_HH__
#define __RESERVOIR_WXL_MRC_HH__

#include <cassert>
#include <vector>
#include <iostream>

using namespace std;

namespace gem5
{
namespace mrcsampler
  {

class MissRatioCurve {
public:
    virtual void reset() = 0;

    virtual void addMissRatio(double mr) = 0;

    virtual void trim(double precision) = 0;

    virtual void print(ostream &os = cout) = 0;
/*
    virtual void print(const char *title) = 0;

    virtual void save(FILE *f, const char *title) = 0;
*/
    virtual double& operator[](int)=0;

    virtual int size()=0;
};

class VectorMRC :public MissRatioCurve {
public:
    vector<double> vmrc;

    VectorMRC(){};
    VectorMRC(int size){
        vmrc.resize(size);
    }
    virtual void reset();

    virtual void addMissRatio(double mr);

    virtual void trim(double precision);

    virtual double& operator[](int posi);

    virtual int size();

    virtual void assign(VectorMRC,int);

    virtual void print(ostream &os = cout) {
        os<<"mrc"<<endl;
        for (double v: vmrc) {
            os << v << endl;
        }
        os<<"end"<<endl;
    }
/*
    virtual void print(const char *title) {
        cout << "this: " << endl;
        cout << "mrc " << title << endl;
        print(cout);
        cout << "mrc " << title << " end" << endl;
    }

    virtual void save(FILE *f, const char *title) {
        fprintf(f, "this: \n");
        fprintf(f, "mrc %s\n", title);
        for (double v: vmrc) {
            fprintf(f, "%lf\n", v);
        }
        fprintf(f, "mrc %s end\n", title);
    }
*/
    
};

  }// namespace sampler
} // namespace gem5
#endif //RESERVOIR_WXL_MRC_H
