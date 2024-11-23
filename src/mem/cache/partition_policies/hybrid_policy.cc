#include "mem/cache/partition_policies/hybrid_policy.hh"

namespace gem5
{
    namespace PartitionPolicy
    {

        hybrid_policy::hybrid_policy(const Params &p) :
         Base(p), cpbm(p.cpbm), min_max(p.min_max), cassoc(p.cassoc)
        {
        }
        AbstractCandidates
        hybrid_policy::filter(
            AbstractCandidates candidates,
             const Addr address, const PARTID partid)
        {
            AbstractCandidates cpbmcan,mmcan,cacan;
            if (cpbm)
            {
                cpbmcan =
                    cpbm->filter(candidates, address, partid);
            }else{
                cpbmcan = candidates;
            }
            assert(cpbmcan.size()>0);
            if (min_max)
            {
                mmcan =
                    min_max->filter(cpbmcan, address, partid);
            }else{
                mmcan = cpbmcan;
            }
            assert(mmcan.size()>0);
            if (cassoc)
            {
                cacan =
                    cassoc->filter(mmcan, address, partid);
            }else{
                cacan = mmcan;
            }
            return cacan;
        }

        uint64_t hybrid_policy::getAvail(
            uint64_t bitmap, const Addr address, const PARTID partid)
        {
            uint64_t cpbm_map = -1, min_max_map = -1, cassoc_map = -1;
            if (cpbm)
            {
                cpbm_map = cpbm->getAvail(bitmap, address, partid);
            }
            if (min_max)
            {
                min_max_map = min_max->getAvail(bitmap, address, partid);
            }
            if (cassoc)
            {
                cassoc_map =
                    cassoc->getAvail(bitmap & cpbm_map, address, partid);
            }
            return cpbm_map & min_max_map & cassoc_map;
        }

        void
        hybrid_policy::update(ReplaceableEntry &infoEntry)
        {
            if (cpbm)
            {
                cpbm->update(infoEntry);
            }
            if (min_max)
            {
                min_max->update(infoEntry);
            }
            if (cassoc)
            {
                cassoc->update(infoEntry);
            }
        }

        void
        hybrid_policy::recycle(ReplaceableEntry &infoEntry)
        {
            if (cpbm)
            {
                cpbm->recycle(infoEntry);
            }
            if (min_max)
            {
                min_max->recycle(infoEntry);
            }
            if (cassoc)
            {
                cassoc->recycle(infoEntry);
            }
        }

        bool 
        hybrid_policy::set(PARTID partid, std::string cmd, int value){
            int ret = 0;
            if ((!ret)&&cpbm)
            {
                ret = cpbm->set(partid,cmd,value);
            }
            if ((!ret)&&min_max)
            {
                ret = min_max->set(partid,cmd,value);
            }
            if ((!ret)&&cassoc)
            {
                ret = cassoc->set(partid,cmd,value);
            }
            return ret;
        }

        int 
        hybrid_policy::get(PARTID partid,std::string cmd){
            int ret = -1;
            if ((ret==-1)&&(cpbm!=NULL))
            {
                ret = cpbm->get(partid,cmd);
            }
            if ((ret==-1)&&(min_max!=NULL))
            {
                ret = min_max->get(partid,cmd);
            }
            if ((ret==-1)&&(cassoc!=NULL))
            {
                ret = cassoc->get(partid,cmd);
            }
            return ret;
        }
    }
}
