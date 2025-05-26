#ifndef __MEM_CACHE_PARTITION_POLICIES_CPBM_HH__
#define __MEM_CACHE_PARTITION_POLICIES_CPBM_HH__

#include <map>

#include "mem/cache/partition_policies/base.hh"
#include "params/CPBM.hh"

namespace gem5
{
    namespace PartitionPolicy
    {
        class CPBM : public Base
        {
//if CPBM is implemented, only candidates that have the bit set are included
//in this case 16-way set associate use 16 bits map to indicate availablity
//Write hits that update the PARTID of a cache line may move
// that line to a different portion
        private:
            std::map<PARTID, int64_t> pbm;

        public:
            typedef CPBMParams Params;
            CPBM(const Params &p);
            virtual AbstractCandidates filter(
                AbstractCandidates candidates,
                 const Addr address, const PARTID partid);

            virtual uint64_t getAvail(
                uint64_t bitmap, const Addr address, const PARTID partid);

            virtual void update(ReplaceableEntry& infoEntry);
            virtual void recycle(ReplaceableEntry& infoEntry);
            virtual bool set(PARTID,std::string,int);
            virtual int get(PARTID,std::string);
        };
    }
}
#endif //__MEM_CACHE_PARTITION_POLICIES_CPBM_HH__
