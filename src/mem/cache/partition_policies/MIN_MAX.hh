#ifndef __MEM_CACHE_PARTITION_POLICIES_CMAX_HH__
#define __MEM_CACHE_PARTITION_POLICIES_CMAX_HH__

#include <map>

#include "mem/cache/partition_policies/base.hh"
#include "params/MIN_MAX.hh"

namespace gem5
{
    namespace PartitionPolicy
    {
        class MIN_MAX : public Base
        {
//If CMAX is implemented and the PARTID currently
//  occupies greater than CMAX
//  fraction of the cache capacity, the choices are limited to
//      softlimit is 0: lines currently occupied by the same PARTID
//           are treated as the set of candidates.
//      softlimit is 1: specifying Unallocated lines, those occupied by a
//          disabled PARTID and those occupied by the same PARTID
//              are treated as the set of candidates.

//MPAMCFG_CMAX.SOFTLIM for PARTID 0 must reset to 0, hard limit behavior.
//TODO differentiate deprecated partid
            bool enable_max,enable_min;
            std::map<PARTID, int> counter;
            std::map<PARTID, int> max_limit;
            std::map<PARTID, int> min_limit;
            std::map<PARTID, bool> softlimit;

        public:
            typedef MIN_MAXParams Params;
            MIN_MAX(const Params &p);
            AbstractCandidates MIN_filter(
                AbstractCandidates candidates,
                 const Addr address, const PARTID partid);
            AbstractCandidates MAX_filter(
                AbstractCandidates candidates,
                const Addr address, const PARTID partid);
            int getPriority(PARTID partid);
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
#endif //__MEM_CACHE_PARTITION_POLICIES_CMAX_HH__
