#ifndef __MEM_CACHE_PARTITION_POLICIES_CASSOC_HH__
#define __MEM_CACHE_PARTITION_POLICIES_CASSOC_HH__

#include <map>

#include "mem/cache/partition_policies/base.hh"
#include "params/CASSOC.hh"

namespace gem5
{
    namespace PartitionPolicy
    {
        class CASSOC : public Base
        {//each set
        //if CASSOC is implemented and the request PARTID currently
        // occupies more than the CASSOC fraction of
        //  the associativity in the unit of associativity
        // that the request addresses, the choices are limited
        //  to only those lines already occupied by the request PARTID.

        private:
            std::map<PARTID, int> limit;
        public:
            typedef CASSOCParams Params;
            CASSOC(const Params &p);
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

#endif //__MEM_CACHE_PARTITION_POLICIES_CASSOCHH__
