
#ifndef __RECORDER_WXL_MRCGENERATOR_HH__
#define __RECORDER_WXL_MRCGENERATOR_HH__


#include <vector>
#include <unordered_map>
#include "mem/mrcsampler/rth.hh"
namespace gem5
{
namespace mrcsampler
  {

using namespace std;

typedef pair<int, int> record;
typedef unordered_multimap<uint64_t, record> ht;
class Recorder {
	vector<uint64_t> timer;
	vector<ht> recorders;
	vector<int> rth;
	vector<int> prerth;
	const int max_reusetime;//MAX_REUSETIME
	const int totalset;//TOTAL_SET
	uint32_t traceidx;
	uint32_t nowcount;
	int count;
	bool complete;
public:
	Recorder(int max_rt, int totalset):max_reusetime(max_rt),totalset(totalset) {
		traceidx = 103;
		reset();
	}

	int compare(){
		nowcount = 0;
		int cnt = 0;
		for (int rt = 0; rt < rth.size(); rt++) {
			int diff = (rth[rt]*count-prerth[rt])>>4;
            cnt += diff*diff;
        }
		//512舍去
		//128以内完成
		if(cnt<128*count*count||count>=4){
			return -1;
		}
		if(cnt>=512*count*count){
			prerth.clear();
			prerth.insert(prerth.begin(), max_reusetime, 0);
			count = 0;
		}
		count ++;
		for(int rt = 0; rt < rth.size(); rt++){
			prerth[rt] += rth[rt];
			rth[rt]=0;
		}
		return 0;
	}

	int insert(uint64_t addr) {
		uint64_t index = (addr>>6)&(totalset-1);
		if(index == traceidx){
			nowcount ++ ;
		}
		if((index&0x7f)!=traceidx){
			return 0;
		}
		timer[index]++;
		ht* recorder = &recorders[index];
		ht::iterator it = recorder->find(addr);
		int rt = 0;
		if (it != recorder->end())
		{
			rt = timer[index] - it->second.first;
			it->second.first = timer[index];
			if (rt >= max_reusetime)
				rt = 0;
		}
		else {
            recorder->insert(pair<uint64_t, record>(addr, record(timer[index], 0)));
		}
		rth[rt]++;
		if(nowcount==64)
			return compare();
		return rt;
	}

	void getrth(ReuseTimeHistogram& RTH){
		for (int rt = 0; rt < rth.size(); rt++) {
            RTH.addReuse(rt, rth[rt]+prerth[rt]);
        }
	}

	void clear_rth() {
		rth.clear();
		rth.insert(rth.begin(), max_reusetime, 0);
		prerth.clear();
		prerth.insert(prerth.begin(), max_reusetime, 0);
	}

	void reset(){
		timer.clear();
		recorders.clear();
		rth.clear();
		prerth.clear();
		timer.insert(timer.begin(), totalset, 0);
        recorders.insert(recorders.begin(), totalset, ht());
		rth.insert(rth.begin(), max_reusetime, 0);
		prerth.insert(prerth.begin(), max_reusetime, 0);
		nowcount = 0;
		count = 0;
		complete = false;
	}
};


  }// namespace sampler
} // namespace gem5
#endif