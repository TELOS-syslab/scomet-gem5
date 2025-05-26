#include "mem/cache/partition_policies/CPBM.hh"

#include "base/trace.hh"
#include "debug/Filter.hh"
#include "debug/SetPartition.hh"

namespace gem5
{
    namespace PartitionPolicy
    {

        CPBM::CPBM(const Params &p) : Base(p)
        {
            pbm[0] = p.part0;
            pbm[1] = p.part1;
            pbm[2] = p.part2;
            pbm[3] = p.part3;
            pbm[4] = p.part4;
            pbm[5] = p.part5;
            pbm[7] = p.part6;
            pbm[8] = p.part8;
            return;
        }

        AbstractCandidates
        CPBM::filter(
            AbstractCandidates candidates, const Addr address,
                const PARTID partid)
        {
            uint64_t bitmap = pbm[partid];
            AbstractCandidates partid_candidates;
            for (auto it = candidates.begin(); it != candidates.end(); it++)
            {
                if ((bitmap >> (*it)->getWay()) & 1)
                {
                    partid_candidates.push_back(*it);
                }
            }
            DPRINTF(Filter,"id:%d, addr:%#x constrain in %#x size:%d\n",
                partid, address, bitmap,partid_candidates.size());

            return partid_candidates;
        }

        uint64_t
        CPBM::getAvail(
            uint64_t bitmap, const Addr address, const PARTID partid)
        {
            return pbm[partid];
        }

        void
        CPBM::update(ReplaceableEntry &infoEntry)
        {
        }

        void
        CPBM::recycle(ReplaceableEntry &infoEntry)
        {
        }

        bool 
        CPBM::set(PARTID partid, std::string cmd, int value){
            if(cmd.compare("cpbm")==0){
                DPRINTF(SetPartition,"cpbm partid:%ld, %#x,pre:%#x\n",partid,value,pbm[partid]);
                pbm[partid] = value;
                return 1;
            }
            return 0;
        }
        
        int 
        CPBM::get(PARTID partid,std::string cmd){
            if(cmd.compare("cpbm")==0){
                return pbm[partid];
            }
            return -1;
        }
    }
}
