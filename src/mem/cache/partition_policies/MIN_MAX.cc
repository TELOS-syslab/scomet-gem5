#include "mem/cache/partition_policies/MIN_MAX.hh"

#include "base/trace.hh"
#include "debug/Filter.hh"
#include "debug/SetPartition.hh"

namespace gem5
{
    namespace PartitionPolicy
    {

        MIN_MAX::MIN_MAX(const Params &p) : Base(p),
        enable_max(p.enable_max),
        enable_min(p.enable_min)
        {
            max_limit[0] = p.max0;
            max_limit[1] = p.max1;
            max_limit[2] = p.max2;
            max_limit[3] = p.max3;
            max_limit[4] = p.max4;
            max_limit[5] = p.max5;
            max_limit[6] = p.max6;
            max_limit[7] = p.max7;
            max_limit[8] = p.max8;

            min_limit[0] = p.min0;
            min_limit[1] = p.min1;
            min_limit[2] = p.min2;
            min_limit[3] = p.min3;
            min_limit[4] = p.min4;
            min_limit[5] = p.min5;
            min_limit[6] = p.min6;
            min_limit[7] = p.min7;
            min_limit[8] = p.min8;

            softlimit[0] = p.softlimit0;
            softlimit[1] = p.softlimit1;
            softlimit[2] = p.softlimit2;
            softlimit[3] = p.softlimit3;
            softlimit[4] = p.softlimit4;
            softlimit[5] = p.softlimit5;
            softlimit[6] = p.softlimit6;
            softlimit[7] = p.softlimit7;
            softlimit[8] = p.softlimit8;

        }

        int
        MIN_MAX::getPriority(PARTID partid)
        {
            if (counter[partid] < min_limit[partid])
            {
                return 4;
            }
            if (!enable_max)
            {
                return 0;
            }
            if (counter[partid] < max_limit[partid])
            {
                return 3;
            }
            //TODO deprecated partid
            return 2;
        }

        AbstractCandidates
        MIN_MAX::MIN_filter(
            AbstractCandidates candidates, const Addr address,
                 const PARTID partid)
        {
            AbstractCandidates partid_candidates;
            int pri = getPriority(partid);
            int min_pri = 5;
            for (auto it = candidates.begin(); it != candidates.end(); it++)
            {
                int it_pri = getPriority((*it)->getPARTID());
                min_pri = it_pri<min_pri ? it_pri : min_pri;
                if (it_pri <= pri)
                {
                    partid_candidates.push_back(*it);
                }
            }
            if (partid_candidates.size()==0){
                DPRINTF(Filter,"id:%d no left, choose victims\n",partid);
                for (auto it = candidates.begin(); it!=candidates.end(); it++)
                {
                    if (getPriority((*it)->getPARTID()) <= min_pri)
                    {
                        partid_candidates.push_back(*it);
                    }
                }
            }
            return partid_candidates;
        }

        AbstractCandidates
        MIN_MAX::MAX_filter(
            AbstractCandidates candidates, const Addr address,
                const PARTID partid)
        {
            AbstractCandidates partid_candidates;
            int min_pri = 5;
            for (auto it = candidates.begin(); it != candidates.end(); it++)
            {
                int it_pri = getPriority((*it)->getPARTID());
                min_pri = it_pri<min_pri ? it_pri : min_pri;
                if ((*it)->getPARTID() == partid)
                {
                    partid_candidates.push_back(*it);
                }
                if (softlimit[partid] &&
                    (((static_cast<ruby::AbstractCacheEntry *>(*it))
                                                        ->m_Permission
                        == gem5::ruby::AccessPermission_NotPresent)))
                {
                    partid_candidates.push_back(*it);
                }
            }
            if (partid_candidates.size()==0){
                DPRINTF(Filter,"id:%d no left, choose victims\n",partid);
                for (auto it = candidates.begin(); it!=candidates.end(); it++)
                {
                    if (getPriority((*it)->getPARTID()) <= min_pri)
                    {
                        partid_candidates.push_back(*it);
                    }
                }
            }
            DPRINTF(Filter,"id:%d addr:%#x candidate size:%d\n",
                 partid,address,partid_candidates.size());
            return partid_candidates;
        }

        AbstractCandidates
        MIN_MAX::filter(
            AbstractCandidates candidates,
             const Addr address, const PARTID partid)
        {
            AbstractCandidates mincan,maxcan;
            if (enable_max && counter[partid] >= max_limit[partid])
            {
                maxcan =
                    MAX_filter(candidates, address, partid);
            }else{
                maxcan = candidates;
            }
            assert(maxcan.size()>0);
            if (enable_min)
            {
                mincan =
                    MIN_filter(maxcan, address, partid);
            }else{
                mincan = maxcan;
            }
            assert(mincan.size()>0);
            return mincan;
        }

        uint64_t
        MIN_MAX::getAvail(uint64_t bitmap,
            const Addr address, const PARTID partid){
            if (enable_max&& counter[partid] >= max_limit[partid]){
                if (!softlimit[partid] && bitmap){
                    DPRINTF(Filter,"id:%d hardlimit, addr:%#x bitmap:%#x\n",
                         partid,address, bitmap);
                    return bitmap;
                }
                if (bitmap == 0){
                    DPRINTF(Filter,"id:%d exceed max, addr:%#x bitmap 0\n",
                         partid,address);
                }
            }
            return -1;
        }

        void
        MIN_MAX::update(ReplaceableEntry &infoEntry)
        {
            counter[infoEntry.getPARTID()]++;
        }

        void
        MIN_MAX::recycle(ReplaceableEntry& infoEntry){
            counter[infoEntry.getPARTID()]--;
        }
        bool 
        MIN_MAX::set(PARTID partid, std::string cmd, int value){
            
            if(cmd.compare("min")==0){
                DPRINTF(SetPartition,"min partid:%ld, %d\n",partid,value);
                min_limit[partid] = value;
                return 1;
            }
            if(cmd.compare("max")==0){
                DPRINTF(SetPartition,"partid:%ld, max:%d, now:%d\n",partid,value,counter[partid]);
                max_limit[partid] = value;
                return 1;
            }
            if(cmd.compare("soft")==0){
                DPRINTF(SetPartition,"soft partid:%ld, %d\n",partid,value);
                softlimit[partid] = value;
                return 1;
            }
            return 0;
        }

        int 
        MIN_MAX::get(PARTID partid,std::string cmd){
            if(cmd.compare("min")==0){
                return min_limit[partid];
            }
            if(cmd.compare("max")==0){
                DPRINTF(SetPartition,"partid:%ld, max:%d, now:%d\n",partid,max_limit[partid],counter[partid]);
                return max_limit[partid];
            }
            if(cmd.compare("soft")==0){
                return softlimit[partid];
            }
            return -1;
        }
    }
}
