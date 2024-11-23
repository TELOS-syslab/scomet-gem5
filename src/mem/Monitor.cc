#include "mem/Monitor.hh"
#include "mem/mrcsampler/mrc.hh"
#include "debug/MRCADJ.hh"
#include "debug/Trace.hh"
#include "base/trace.hh"
#include <cmath>

namespace gem5{
using namespace PartitionPolicy;
using namespace mrcsampler;
using namespace std;

Monitor::Monitor(const MonitorParams &p):
    SimObject(p),timeout(p.timeout),traceID(0),
    premrc(p.cacheway),mrcgen(p.mrcgen),
    partition(p.partition),cacheway(p.cacheway),
    penalty(p.penalty),l3_assoc(p.l3_assoc),l3_set(p.l3_set),
    EcheckMR([this]{ checkMR(); },name() + ".checkMR"),
    EbuildMR([this]{ buildMR(); },name() + ".buildMR"){
        if(partition)
            schedule(EcheckMR,curTick() +10*mrcMS);
        if(debug::Trace){
            trace = simout.create("trace");
        }
    }

AbstractCandidates
Monitor::filter(//get partid bitmap and return available bitmap
    AbstractCandidates candidates, const Addr address,
        const PARTID partid){
            if(partition==NULL){
                return candidates;
            }
        return partition->filter(candidates,address,partid);
        }

uint64_t
Monitor::getAvail(
    uint64_t bitmap, const Addr address, const PARTID partid){
        if(partition==NULL){
            return -1;
        }
    return partition->getAvail(bitmap,address,partid);
    }

void Monitor::update(ReplaceableEntry& infoEntry){
    if(partition)
        partition->update(infoEntry);
}
void Monitor::recycle(ReplaceableEntry& infoEntry){
    if(partition)
        partition->recycle(infoEntry);
}

int 
Monitor::cachehit(PARTID partid, Addr address){
    auto it = mrcstates.find(partid);
    if(it == mrcstates.end()){
        mrcstates[partid] = mrcState();
        mrcstates[partid].os = simout.create("partid:"+to_string(partid));
    }
    if(debug::Trace){
        ostream* output = trace->stream();
        (*output)<<curTick()<<" "<<partid<<" "<<address<<" 1\n";
    }
    mrcstates[partid].hit++;
    mrcstates[partid].addrset.insert(address);
    int ret = 0;
    if(partid==traceID&&mrcgen){
        ret = mrcgen->trace(address);
        if(ret == -1){
            if(EbuildMR.scheduled()){
                reschedule(EbuildMR,curTick());
            }else{
                schedule(EbuildMR,curTick());
            }
        }
    }
    return 0;
}

int 
Monitor::cachemiss(PARTID partid,Addr address){
    auto it = mrcstates.find(partid);
    if(it == mrcstates.end()){
        mrcstates[partid] = mrcState();
        mrcstates[partid].os = simout.create("partid:"+to_string(partid));
    }
    if(debug::Trace){
        ostream* output = trace->stream();
        (*output)<<curTick()<<" "<<partid<<" "<<address<<" 0\n";
    }
    mrcstates[partid].miss++;
    mrcstates[partid].addrset.insert(address);
    int ret = 0;
    if(partid==traceID&&mrcgen){
        ret = mrcgen->trace(address);
        if(ret == -1){
            if(EbuildMR.scheduled()){
                reschedule(EbuildMR,curTick());
            }else{
                schedule(EbuildMR,curTick());
            }
        }
    }
    return 0;
}

void 
Monitor::checkMR(){
    schedule(EcheckMR, curTick() +10*mrcMS);
    if(mrcstates.size() == 0){
        return;
    }
    for(auto& it:mrcstates){
        it.second.total = it.second.hit + it.second.miss;
        if(it.second.total == 0) continue;
        double rate = it.second.hit/it.second.total;
        DPRINTF(MRCADJ,"id:%ld,ratio:%lf,pre:%lf,acc:%lf,fp:%d\n"
            ,it.first,rate,it.second.preRatio,it.second.total,
            it.second.addrset.size());
        int maxpart = partition->get(it.first,"max");
        it.second.hit = 0;
        it.second.miss = 0;
        it.second.addrset.clear();
        /*
        if(abs(rate-it.second.preRatio)>5e-2||
        abs(rate+it.second.idealRatio-1)>4e-2){
            if(it.second.state==STABLE){
                //not stable
                it.second.state=FLUC;
            }
            if(it.second.state==TIMEOUT){
                //time and vary
                it.second.state=TFLUC;
            }
            it.second.preRatio=rate;
        }
        if(it.second.state==MRC){
            //get new stable miss rate
            it.second.state=STABLE;
        }
        */
    }
    
    if(mrcgen&&traceID==0&&(!EbuildMR.scheduled())){
        schedule(EbuildMR,curTick());
    }
}
    
void
Monitor::printmrc(){
    // switch to file
    //mrcgen->printMRC();
    ostream* output = mrcstates[traceID].os->stream();
    (*output) <<"start:"<<mrcstates[traceID].time<< " end:"<<curTick() <<endl;
    mrcstates[traceID].mrc.print(*output);
    (*output) << endl;
}

void 
Monitor::buildMR(){
    if(traceID!=0){
        //idle = curTick();
        VectorMRC mrc;
        if(!mrcgen->mrc(mrc)){
            warn("%ld,partid%ld no mrc\n",curTick(),traceID);
            schedule(EbuildMR, curTick() +10*mrcMS);
            return;
        }
        //generate mrc
        if(!mrcstates[traceID].avail||
            abs(mrcstates[traceID].mrc[cacheway]-mrc[cacheway])>1e-2){
            mrcstates[traceID].avail = 1;
            mrcstates[traceID].state=MRC;
            (mrcstates[traceID].mrc).assign(mrc,cacheway+1);
            adjust();
        }
        /*
        if(mrcstates[traceID].state==WARMUP){
            premrc.assign(mrc,cacheway+1);
            mrcstates[traceID].state=CHECK;
            mrcstates[traceID].time+=20;
            schedule(EbuildMR, curTick() +20*mrcMS);
            return;
        }else if(mrcstates[traceID].state==CHECK||
                mrcstates[traceID].state==RECHECK||
                mrcstates[traceID].state==TCHECK){
            if(abs(mrc[cacheway]-premrc[cacheway])<3e-2){
                (mrcstates[traceID].mrc).assign(mrc,cacheway+1);
                mrcstates[traceID].state=MRC;
                mrcstates[traceID].avail = 1;
                //once get mrc adjust
                adjust();
            }else{
                //use mean to adjust
                if(mrcstates[traceID].time<timeout){
                    premrc.assign(mrc,cacheway+1);
                    mrcstates[traceID].time+=5;
                    schedule(EbuildMR, curTick() +5*mrcMS);
                    return;
                }
                mrcstates[traceID].avail = 1;
                (mrcstates[traceID].mrc).assign(mrc,cacheway+1);
                mrcstates[traceID].state=TIMEOUT;
                adjust();
            }
        }
        */
    }

    for(auto& it:mrcstates){
        if(it.second.state==NEWID){
            mrcgen->reset();
            traceID = it.first;
            mrcstates[traceID].time=curTick();
            it.second.state=WARMUP;
            schedule(EbuildMR, curTick() +30*mrcMS);
            return;
        }
    }
    
    for(auto& it:mrcstates){
        if(it.second.state==FLUC){
            mrcgen->reset();
            traceID = it.first;
            mrcstates[traceID].time=curTick();
            it.second.state=RECHECK;
            premrc.assign(it.second.mrc,cacheway+1);
            schedule(EbuildMR, curTick() +20*mrcMS);
            return;
        }
    }

    for(auto& it:mrcstates){
        if(it.second.state==TFLUC){
            mrcgen->reset();
            traceID = it.first;
            mrcstates[traceID].time=curTick();
            it.second.state=TCHECK;
            premrc.assign(it.second.mrc,cacheway+1);
            schedule(EbuildMR, curTick() +20*mrcMS);
            return;
        }
    }

    for(auto& it:mrcstates){
        if(it.second.state==TIMEOUT){
            mrcgen->reset();
            traceID = it.first;
            mrcstates[traceID].time=curTick();
            it.second.state=TCHECK;
            premrc.assign(it.second.mrc,cacheway+1);
            schedule(EbuildMR, curTick() +20*mrcMS);
            return;
        }
    }
/*
    if(curTick()-idle>40*mrcMS){
        for(auto& it:mrcstates){
            it.second.state=FLUC;
        }
    }
    */
    traceID = 1;//hardset id1 step
    mrcstates[traceID].time=curTick();
    mrcgen->reset();
}


void
Monitor::adjust(){
    if(debug::MRCADJ){
        printmrc();
    }
    if(partition == NULL || mrcstates.size() == 0){
        return;
    }//非激进调整，主要调整cpbm
    int idsize = mrcstates.size();
    vector<PARTID> idtable = vector<PARTID>(idsize,0);
    vector<vector<double>> besttargettable = vector<vector<double>>(idsize,vector<double>());
    vector<vector<int>> bestidx = vector<vector<int>>(idsize,vector<int>(cacheway+1,0));
    int idx = 0;
    for(auto it:mrcstates){
        if(it.second.avail){
            idtable[idx] = it.first;
            transform(it.second.mrc.vmrc.begin(),it.second.mrc.vmrc.end(),back_inserter(besttargettable[idx]),
            [&](double value){
                return (value*penalty[idx]);});
            //bestmrctable[idx].assign(it.second.mrc.vmrc.begin(),it.second.mrc.vmrc.end());
        }else{
            DPRINTF(MRCADJ,"partid:%d not avail\n",it.first);
            return;
            idtable[idx] = 0;
            besttargettable[idx] = vector<double>(cacheway+1,0);
        }
        idx++;
    }
    idx = 0;
    for(int j = 0; j <= cacheway; j++){
        bestidx[idx][j]=j;
    }
    //get max ipc
    for(idx = 1; idx < idsize; idx ++){
        for(int j = cacheway ; j >= 0; j --){
            int maxid = -1;
            double minmr = idsize*100;
            for(int i = 0; i <= j; i++){
                if(besttargettable[idx][i]+besttargettable[idx-1][j-i]<minmr){
                    maxid = i;
                    minmr = besttargettable[idx][i]+besttargettable[idx-1][j-i];
                }
            }
            bestidx[idx][j] = maxid;
            besttargettable[idx][j]=minmr;
        }
    }
    int rest = cacheway;
    for(idx = idsize -1 ; idx >= 0; idx --){
        int maxpart = partition->get(idtable[idx],"max");
        int minpart = partition->get(idtable[idx],"min");
        
        int objmin = bestidx[idx][rest];
        DPRINTF(MRCADJ,"partid:%d ideal:%lfMB missRatio:%lf\n",idtable[idx],objmin*mrcgen->getSet()*64.0/1024/1024,mrcstates[idtable[idx]].mrc[objmin]);
        mrcstates[idtable[idx]].idealRatio = mrcstates[idtable[idx]].mrc[objmin];
        objmin*=mrcgen->getSet();
        int objmax = objmin*1.05;
        objmin *= 0.95;
        partition->set(idtable[idx],"max",objmax);
        partition->set(idtable[idx],"min",objmin);
        rest -= bestidx[idx][rest];
        continue;

        //todo max var 10%
        //min set 90% performance
        // adjust cpbm according var(?)
        maxpart = partition->get(idtable[idx],"max");
        minpart = partition->get(idtable[idx],"min");
        
        objmin = bestidx[idx][rest];
        DPRINTF(MRCADJ,"partid:%d ideal:%lfMB missRatio:%lf\n",idtable[idx],objmin*mrcgen->getSet()*64.0/1024/1024,mrcstates[idtable[idx]].mrc[objmin]);
        mrcstates[idtable[idx]].idealRatio = mrcstates[idtable[idx]].mrc[objmin];
        objmax = objmin*mrcgen->getSet()*1.1;
        if(maxpart>objmax){
            maxpart*=0.9;
            objmax =max(maxpart,objmax);
        }else{
            maxpart*=1.1;
            objmax =min(maxpart,objmax);
        }
        objmax=min(objmax,l3_set*l3_assoc);
        partition->set(idtable[idx],"max",objmax);
        
        double mr = mrcstates[idtable[idx]].mrc.vmrc[objmin];
        mr *= 1.1;
        objmin--;
        while(objmin>=0&&mrcstates[idtable[idx]].mrc.vmrc[objmin]<=mr){
            objmin--;
        }
        objmin++;
        objmin*=mrcgen->getSet();
        if(minpart>objmin){
            minpart*=0.9;
            objmin = max(minpart,objmin);
        }else{
            minpart*=1.1;
            objmin = min(minpart,objmin);
        }
        objmin=max(objmin,1024);
        partition->set(idtable[idx],"min",objmin);
        assert(objmin<=objmax);
        //adjust cpbm
        objmax*=1.1;
        objmin/=l3_set;
        objmax=(objmax+l3_set)/l3_set;
        int cpbm = partition->get(idtable[idx],"cpbm");
        int now = bitcount(cpbm);
        if(now<=objmin||now<objmax){
            //add
            cpbm |= (cpbm<<1)|(cpbm>>1);
        }else if(now>objmax+1){
            cpbm&= ((cpbm<<1)&(cpbm>>1))|1|(1<<(l3_assoc-1));
        }
        cpbm &= (1<<l3_assoc)-1;
        partition->set(idtable[idx],"cpbm",cpbm);
        rest -= bestidx[idx][rest];
    }
    
}

}
