#include "mem/cache/partition_policies/base.hh"

namespace gem5
{
    namespace PartitionPolicy
    {
        int bitcount(uint64_t n)
        {
            n = COUNT(n, 0);
            n = COUNT(n, 1);
            n = COUNT(n, 2);
            n = COUNT(n, 3);
            n = COUNT(n, 4);
            n = COUNT(n, 5);
            return n;
        }
    }
}