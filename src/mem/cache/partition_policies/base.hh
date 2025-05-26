
#ifndef __MEM_CACHE_PARTITION_POLICIES_BASE_HH__
#define __MEM_CACHE_PARTITION_POLICIES_BASE_HH__

#include <memory>

#include "base/compiler.hh"
#include "mem/cache/replacement_policies/base.hh"
#include "mem/cache/replacement_policies/replaceable_entry.hh"
#include "mem/packet.hh"
#include "mem/ruby/protocol/AccessPermission.hh"
#include "mem/ruby/slicc_interface/AbstractCacheEntry.hh"
#include "params/BasePartitionPolicy.hh"
#include "sim/sim_object.hh"
#include <string.h>

namespace gem5
{
  namespace PartitionPolicy
  {
#define COUNT(x, c) (((x)&MASK##c) + (((x) >> (1 << c)) & MASK##c))

    uint64_t const MASK0 = 0x5555555555555555;
    uint64_t const MASK1 = 0x3333333333333333;
    uint64_t const MASK2 = 0x0f0f0f0f0f0f0f0f;
    uint64_t const MASK3 = 0x00ff00ff00ff00ff;
    uint64_t const MASK4 = 0x0000ffff0000ffff;
    uint64_t const MASK5 = 0x00000000ffffffff;


    int bitcount(uint64_t n);
    typedef std::vector<ruby::AbstractCacheEntry *> AbstractCandidates;
    /**
     * A common base class of cache replacement policy objects.
     */
    class Base : public SimObject
    {
    public:
      typedef BasePartitionPolicyParams Params;
      Base(const Params &p) : SimObject(p) {}
      virtual ~Base() = default;

      /**
       * Find available victim among candidates.
       *
       * @param candidates Replacement candidates, selected by policy.
       * @return Replacement entry to be replaced.
       */
      virtual AbstractCandidates filter(
          AbstractCandidates candidates, const Addr address,
             const PARTID partid) = 0;
          //get partid bitmap and return available bitmap
      virtual uint64_t getAvail(
          uint64_t bitmap, const Addr address, const PARTID partid) = 0;

      virtual void update(ReplaceableEntry& infoEntry) = 0;
      virtual void recycle(ReplaceableEntry& infoEntry) =0;
      virtual bool set(PARTID,std::string,int)=0;
      virtual int get(PARTID,std::string)=0;
    };

  } // namespace partition_policy
} // namespace gem5

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_BASE_HH__
