#include "mem/cache/partition_policies/CASSOC.hh"

#include "base/trace.hh"
#include "debug/Filter.hh"
#include "debug/SetPartition.hh"
#include "params/CASSOC.hh"

namespace gem5
{
    namespace PartitionPolicy
    {

        CASSOC::CASSOC(const Params &p) : Base(p)
        {
            limit[0] = p.part0;
            limit[1] = p.part1;
            limit[2] = p.part2;
            limit[3] = p.part3;
            limit[4] = p.part4;
            limit[5] = p.part5;
            limit[7] = p.part6;
            limit[8] = p.part8;
            return;
        }

        AbstractCandidates
        CASSOC::filter(
            AbstractCandidates candidates, const Addr address,
            const PARTID partid)
        {
            AbstractCandidates partid_candidates;
            int max_assoc = limit[partid];
            int count = 0;
            for (auto it = candidates.begin(); it != candidates.end(); it++)
            {
                if ((*it)->getPARTID() == partid)
                {
                    partid_candidates.push_back(*it);
                    count++;
                }
            }
            if (count < max_assoc)
            {
                partid_candidates = candidates;
            }
            DPRINTF(Filter, "id:%d, addr:%#x count:%d max:%d,size%d\n",
                    partid, address, count, max_assoc,partid_candidates.size());
            return partid_candidates;
        }

        uint64_t CASSOC::getAvail(
            uint64_t bitmap, const Addr address, const PARTID partid)
        {
            if (bitcount(bitmap) < limit[partid])
            {
                return -1;
            }
            return bitmap;
        }

        void
        CASSOC::update(ReplaceableEntry &infoEntry)
        {
        }

        void
        CASSOC::recycle(ReplaceableEntry &infoEntry)
        {
        }

        bool
        CASSOC::set(PARTID partid, std::string cmd, int value)
        {
            if (cmd.compare("cassoc") == 0)
            {
                DPRINTF(SetPartition, "cassosc partid:%ld, %d\n", partid, value);
                limit[partid] = value;
                return 1;
            }
            return 0;
        }

        int
        CASSOC::get(PARTID partid, std::string cmd)
        {
            if (cmd.compare("cassoc") == 0)
            {
                return limit[partid];
            }
            return -1;
        }
    }
}
