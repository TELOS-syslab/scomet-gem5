#ifndef __MEM_CACHE_PARTITION_POLICIES_HYBRID_HH__
#define __MEM_CACHE_PARTITION_POLICIES_HYBRID_HH__

#include "mem/cache/partition_policies/CASSOC.hh"
#include "mem/cache/partition_policies/CPBM.hh"
#include "mem/cache/partition_policies/base.hh"

#include "mem/cache/partition_policies/MIN_MAX.hh"
#include "params/HybridPolicy.hh"

namespace gem5
{
    namespace PartitionPolicy
    {
        class hybrid_policy : public Base
        {
//if CPBM is implemented, only candidates that have the bit set are included
//If CMAX is implemented and the PARTID currently occupies greater than CMAX
//  fraction of the cache capacity, the choices are limited to
//      softlimit is 0: lines currently occupied by the same PARTID
//                                   are treated as the set of candidates.
//      softlimit is 1: specifying Unallocated lines, those occupied by a
//          disabled PARTID and those occupied by the same PARTID
//                                  are treated as the set of candidates.
//if CASSOC is implemented and the request PARTID currently occupies
//  more than the CASSOC fraction of the associativity in the unit of
// associativity that the request addresses, the choices are limited
//  to only those lines already occupied by the request PARTID.
        private:
            CPBM* cpbm;
            MIN_MAX* min_max;
            CASSOC* cassoc;
        public:
            typedef HybridPolicyParams Params;
            hybrid_policy(const Params &p);
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
#endif //__MEM_CACHE_PARTITION_POLICIES_HYBRID_HH__
