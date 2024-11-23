#include "mem/mem_bandwidth_ctrl.hh"

#include "base/compiler.hh"
#include "base/random.hh"
#include "debug/MemBandwidthCtrl.hh"
#include "debug/MemBWCtrlQueue.hh"
#include "debug/MemBWCalc.hh"
#include "debug/MemTime.hh"
#include "debug/CriticalReport.hh"
#include "sim/system.hh"


namespace gem5
{

MemBandwidthCtrl::MemBandwidthCtrl(const MemBandwidthCtrlParams &params) :
    ClockedObject(params),
    cpuPort(params.name + ".cpu_side",this),
    memPort(params.name + ".mem_side", this),
    QoS_Queue_Len_HH(params.QoS_Queue_Len_HH),
    QoS_Queue_Len_H(params.QoS_Queue_Len_H),
    QoS_Queue_Len_M(params.QoS_Queue_Len_M),
    QoS_Queue_Len_L(params.QoS_Queue_Len_L),
    Write_Threshold(params.Write_Threshold),
    Time_Threshold(params.Time_Threshold),
    Partition_Mode(params.Partition_Mode),
    TDM_Len_Max(params.TDM_Len_Max),
    BWCtrl_State(BWCTRL_READ_STATE),
    Mem_Bandwidth_Monitor_Window(params.Mem_Bandwidth_Monitor_Window),
    Last_Window(0),
    nextReqEvent([this]{ processNextReqEvent(); }, name()),
    respondEvent([this]{ processRespondEvent(); }, name()),
    stats(this)
{
    Time_Threshold *= 1000;
    BandwidthCounter = 0;
    LastCountTime = 0;
    DPRINTF(MemBandwidthCtrl, "Time Threshold%llu\n", Time_Threshold);
    for (int i = 0; i < QoS_Type; i++) {
        Cur_Len[i] = 0;
        Resp_Len[i] = 0;
        Read_Len[i] = 0;
        Write_Len[i] = 0;
        MemBWCtrl_Req_queue[i].clear();
        MemBWCtrl_Resp_queue[i].clear();
        MemBWCtrl_Read_queue[i].clear();
        MemBWCtrl_Write_queue[i].clear();
    }
    
    for (int i = 0; i < PARTIDNUM; i++) {
        MonBandwidth[i] = 0;
        MBMAX[i] = 100;
        MBMIN[i] = 0;
        PRI[i] = 0;
        MemBWCtrl_Req_TDM_queue[i].clear();
        MemBWCtrl_Resp_TDM_queue[i].clear();
    }

    QoS_Len[0] = QoS_Queue_Len_HH;
    QoS_Len[1] = QoS_Queue_Len_H;
    QoS_Len[2] = QoS_Queue_Len_M;
    QoS_Len[3]= QoS_Queue_Len_L;
    cnt1 = 0;
    cnt2 = 0;
    avgDeal = 0, avgBack = 0;
    //printf("%d %d %d %d\n", QoS_Queue_Len_HH, QoS_Queue_Len_H, QoS_Queue_Len_M, QoS_Queue_Len_L);
}

Port &
MemBandwidthCtrl::getPort(const std::string &if_name, PortID idx)
{
    // This is the name from the Python SimObject declaration in MemBandwidthCtrl.py
    if (if_name == "mem_side") {
        panic_if(idx != InvalidPortID,
                 "Mem side of simple cache not a vector port");
        return memPort;
    } else if (if_name == "cpu_side") {
        panic_if(idx != InvalidPortID,
                 "CPU side of simple cache not a vector port");
        // We should have already created all of the ports in the constructor
        return cpuPort;
    } else {
        // pass it along to our super class
        return ClockedObject::getPort(if_name, idx);
    }
}

bool
MemBandwidthCtrl::CPUSidePort::sendPacket(PacketPtr pkt)
{
    // Note: This flow control is very simple since the cache is blocking.

    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    // If we can't send the packet across the port, store it for later.
    DPRINTF(MemBandwidthCtrl, "Sending %s to CPU\n", pkt->print());
    if (!sendTimingResp(pkt)) {
        DPRINTF(MemBandwidthCtrl, "failed!\n");
        blockedPacket = pkt;
        return false;
    }
    /*if (owner->PacketTimer[pkt->getAddr()]) {
        Tick TotalTime = curTick() - owner->PacketTimer[pkt->getAddr()];
        //printf("Total Time %llu\n", TotalTime);
        owner->BackTime[owner->cnt2] = TotalTime;
        owner->avgBack += TotalTime;
        owner->cnt2++;
        owner->PacketTimer.erase(owner->PacketTimer.find(pkt->getAddr()));
        if (owner->cnt2 == 10000) {
            std::sort(owner->BackTime, owner->BackTime + owner->cnt2);
            DPRINTF(MemTime, "Avg Back Time: %.4lf\n", 1.0 * owner->avgBack / owner->cnt2);
            DPRINTF(MemTime, "95 Deal Time: %.4lf\n", 1.0 * owner->BackTime[9500]);
            DPRINTF(MemTime, "99 Deal Time: %.4lf\n", 1.0 * owner->BackTime[9900]);
            owner->cnt2 = 0;
            owner->avgBack = 0;
        }
    }*/
    return true;
}

AddrRangeList
MemBandwidthCtrl::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

void
MemBandwidthCtrl::CPUSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) {
        // Only send a retry if the port is now completely free
        needRetry = false;
        DPRINTF(MemBandwidthCtrl, "Sending retry req.\n");
        sendRetryReq();
    }
}

Tick 
MemBandwidthCtrl::CPUSidePort::recvAtomic(PacketPtr pkt) {
    //TODO atomic request return latency
    return owner->handleAtomic(pkt);
}

void
MemBandwidthCtrl::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    // Just forward to the cache.
    return owner->handleFunctional(pkt);
}

bool
MemBandwidthCtrl::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    DPRINTF(MemBandwidthCtrl, "Got request %s\n", pkt->print());
    if (needRetry) {
        // pay attention to dead block
        //blocked means peer can't do anymore
        //needretry means we can't deal it
        DPRINTF(MemBandwidthCtrl, "Request blocked\n");
        needRetry = true;
        return false;
    }
    // Just forward to the cache.
    if (!owner->handleRequest(pkt)) {
        DPRINTF(MemBandwidthCtrl, "Request failed\n");
        // stalling
        needRetry = true;
        return false;
    } else {
        DPRINTF(MemBandwidthCtrl, "Request succeeded\n");
        return true;
    }
}

void
MemBandwidthCtrl::CPUSidePort::recvRespRetry()
{
    // We should have a blocked packet if this function is called.
    assert(blockedPacket != nullptr);

    // Grab the blocked packet.
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    DPRINTF(MemBandwidthCtrl, "Retrying response pkt %s\n", pkt->print());
    // Try to resend it. It's possible that it fails again.
    //sendPacket(pkt);
    owner->schedule(owner->respondEvent, curTick());
    // We may now be able to accept new packets
    //trySendRetry();
}

bool
MemBandwidthCtrl::MemSidePort::sendPacket(PacketPtr pkt)
{
    // Note: This flow control is very simple since the cache is blocking.

    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    // If we can't send the packet across the port, store it for later.
    if (!sendTimingReq(pkt)) {
        //send failed block it until recvRetry
        blockedPacket = pkt;
        return false;
    }
    owner->BandwidthCounter += pkt->getSize();
    //if (pkt->qosValue() > 0)
    //    DPRINTF(MemBWCalc, "Packet Addr %llx\n", pkt->getAddr());
    if (curTick() - owner->LastCountTime > 1000000000) {
        DPRINTF(MemBWCalc, "Current Bandwidth %.05lfMB/ms\n", 
                1.0 * owner->BandwidthCounter / 1024 / 1024 );
        owner->LastCountTime = curTick();
        owner->BandwidthCounter = 0;
    }
    /*if (owner->PacketTimer[pkt->getAddr()]) {
        Tick QueueTime = curTick() - owner->PacketTimer[pkt->getAddr()];
        //printf("Deal Time %llu\n", QueueTime);
        owner->DealTime[owner->cnt1] = QueueTime;
        owner->cnt1++;
        owner->avgDeal += QueueTime;
        if (owner->cnt1 == 10000) {
            std::sort(owner->DealTime, owner->DealTime + owner->cnt1);
            DPRINTF(MemTime, "Avg Deal Time: %.4lf\n", 1.0 * owner->avgDeal / owner->cnt1);
            DPRINTF(MemTime, "95 Deal Time: %.4lf\n", 1.0 * owner->DealTime[9500]);
            DPRINTF(MemTime, "95 Deal Time: %.4lf\n", 1.0 * owner->DealTime[9900]);
            owner->cnt1 = 0;
            owner->avgDeal = 0;
        }
    }*/
    return true;
}

void
MemBandwidthCtrl::MemSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) {
        // Only send a retry if the port is now completely free
        needRetry = false;
        DPRINTF(MemBandwidthCtrl, "Sending retry req.\n");
        sendRetryResp();
    }
}

bool
MemBandwidthCtrl::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    DPRINTF(MemBandwidthCtrl, "Got response %s\n", pkt->print());
    if (pkt->qosValue() > 0) {
        DPRINTF(CriticalReport, "Resp %s\n", pkt->print());
    }
    if (needRetry) {
        // pay attention to dead block
        //blocked means peer can't do anymore
        //needretry means we can't deal it
        DPRINTF(MemBandwidthCtrl, "Response blocked\n");
        needRetry = true;
        return false;
    }
    // Just forward to the cache.
    if (!owner->handleResponse(pkt)) {
        DPRINTF(MemBandwidthCtrl, "Response failed\n");
        // stalling
        needRetry = true;
        return false;
    } else {
        DPRINTF(MemBandwidthCtrl, "Response succeeded\n");
        return true;
    }
}

void
MemBandwidthCtrl::MemSidePort::recvReqRetry()
{
    // We should have a blocked packet if this function is called.
    assert(blockedPacket != nullptr);

    // Grab the blocked packet.
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    DPRINTF(MemBandwidthCtrl, "Retrying request pkt %s\n", pkt->print());

    // Try to resend it. It's possible that it fails again.
    owner->schedule(owner->nextReqEvent, curTick());
    //sendPacket(pkt);
}

void
MemBandwidthCtrl::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

bool
MemBandwidthCtrl::handleRequest(PacketPtr pkt)
{
    //if we can deal packet return true else return false
    DPRINTF(MemBandwidthCtrl, "Got request for addr %#x\n", pkt->getAddr());
    //TODO handle request
    //////////////////////////////////////////////////////////////////////////////
    assert(Partition_Mode == BW_Default || Partition_Mode == BW_MINMAX || 
           Partition_Mode == BW_Portion || Partition_Mode == BW_Priority ||
           Partition_Mode == BW_RW_Split);

    if (pkt->qosValue() > 0) {
        DPRINTF(CriticalReport, "%s\n", pkt->print());
    }
    /*PacketDataPtr data = pkt->getPtr<uint8_t>();
    int ll = pkt->getSize();
    printf("Addr %#x Data ", pkt->getAddr());
    for (int i = 0; i < ll; i++)
        printf("%c", data[i]);
    std::cout << std::endl << pkt->print() << std::endl;*/
    if (Partition_Mode == BW_RW_Split) {
        if (pkt->isWrite()) {
            auto it = MemBWCtrl_Write_queue[0].begin();
            Addr addr = pkt->getAddr();
            unsigned int size = pkt->getSize();
            for (const auto& WritePkt : MemBWCtrl_Write_queue[0]) {
                if (addr <= WritePkt->getAddr() && WritePkt->getAddr() + WritePkt->getSize() <= addr + size)
                    break;
                it++;
            }
            if (it != MemBWCtrl_Write_queue[0].end()) {
                MemBWCtrl_Write_queue[0].erase(it);
                Write_Len[0]--;
            }
            if (Write_Len[0] < QoS_Queue_Len_HH) {
                Write_Len[0]++;
                DPRINTF(MemBWCtrlQueue, "Write Len %d\n", Write_Len[0]);
                MemBWCtrl_Write_queue[0].push_back(pkt);
                if (Write_Len[0] >= Write_Threshold) {
                    BWCtrl_State = BWCTRL_WRITE_STATE;
                }
                /*if (curTick() > Time_Threshold)
                    Write_Threshold = 1;*/
            } else {
                return false;
            }
        } else {
            panic_if(!pkt->isRead(), "Packet should be Read or Write\n");
            Addr addr = pkt->getAddr();
            unsigned int size = pkt->getSize();

            //WPkt is the Packet can serve to the readrequest
            PacketPtr Wpkt = nullptr;

            for (const auto& WritePkt : MemBWCtrl_Write_queue[0]) {
                if (WritePkt->getAddr() <= addr && addr + size <= WritePkt->getAddr() + WritePkt->getSize())
                    Wpkt = WritePkt;
            }

            if (Wpkt != nullptr && Resp_Len[0] < QoS_Queue_Len_HH) {
                PacketDataPtr Wdata = Wpkt->getPtr<uint8_t>() + (addr - Wpkt->getAddr());
                PacketDataPtr Rdata = pkt->getPtr<uint8_t>();
                for (int i = 0; i < size; i++) {
                    uint8_t val = Wdata[i];
                    Rdata[i] = val;
                }  
                pkt->makeResponse();
                handleResponse(pkt);
                DPRINTF(MemBandwidthCtrl, "Pkt %s\n serviced by Write Queue.\n", pkt->print());
                return true;
            }

            if (Read_Len[0] < QoS_Queue_Len_HH) {
                Read_Len[0]++;
                DPRINTF(MemBWCtrlQueue, "Read Len %d\n", Read_Len[0]);
                MemBWCtrl_Read_queue[0].push_back(pkt);
            } else {
                return false;
            }
        }
    }   

    if (Partition_Mode == BW_Default) {
        if (Cur_Len[0] < QoS_Queue_Len_HH) {
            Cur_Len[0]++;
            DPRINTF(MemBWCtrlQueue, "Req Len %d\n", Cur_Len[0]);
            MemBWCtrl_Req_queue[0].push_back(pkt);
        } else {
            return false;
        }
    }

    if (Partition_Mode == BW_Priority) {
        PARTID BW_partid = pkt->requestorId();
        int Pri = 1 - pkt->qosValue();

        //printf("Mem QoS %d\n", pkt->qosValue());
        while (Cur_Len[Pri] == QoS_Len[Pri] && Pri < QoS_Type) 
            Pri++;
        //printf("%d\n", Pri);
        if (Pri < QoS_Type) {
            Cur_Len[Pri]++;
            MemBWCtrl_Req_queue[Pri].push_back(pkt);
        } else {
            return false;
        }

        /*if (pkt->qosValue()) {
            PacketTimer[pkt->getAddr()] = curTick();
            //printf("%llu %llu\n", pkt->getAddr(), curTick());
        }*/
        if (Cur_Len[1] + Cur_Len[2] + Cur_Len[3] > 1) {
            DPRINTF(MemBWCtrlQueue, "MemBandwidthCtrl Queue QoS 1 Len %d\n", Cur_Len[0]);
            DPRINTF(MemBWCtrlQueue, "MemBandwidthCtrl Queue QoS 0 Len %d\n", Cur_Len[1] + Cur_Len[2] + Cur_Len[3]);
        }

        //pkt->qosValue(0);
    }

    if (Partition_Mode == BW_Portion) {
        PARTID BW_partid = pkt->requestorId();
        if (Cur_TDM_Len[BW_partid] < TDM_Len_Max) {
            Cur_TDM_Len[BW_partid]++;
            MemBWCtrl_Req_TDM_queue[BW_partid].push_back(pkt);
        } else {
            return false;
        }
    }

    if (Partition_Mode == BW_MINMAX) {
        PARTID BW_partid = pkt->requestorId();
        int Pri = PRI[BW_partid];
        while (Cur_Len[Pri] == QoS_Len[Pri] && Pri < QoS_Type) 
            Pri++;
        if (Pri < QoS_Type) {
            Cur_Len[Pri]++;
            MemBWCtrl_Req_queue[Pri].push_back(pkt);
        } else {
            return false;
        }
    }
    // Schedule an event after cache access latency to actually access
    if (memPort.getblockedPacket() == nullptr && !nextReqEvent.scheduled())
        schedule(nextReqEvent, curTick());
    //schedule or just deal packet
    //cpuPort.trySendRetry();
    //memPort.trySendRetry();
    //////////////////////////////////////////////////////////////////////////////
    return true;
}

bool
MemBandwidthCtrl::handleResponse(PacketPtr pkt)
{
    //if we can deal packet return true else return false
    DPRINTF(MemBandwidthCtrl, "Got response for addr %#x\n", pkt->getAddr());
    //TODO handle request
    //////////////////////////////////////////////////////////////////////////////
    if (Resp_Len[0] < QoS_Queue_Len_HH) {
        Resp_Len[0]++;
        DPRINTF(MemBWCtrlQueue, "Resp Len %d\n", Resp_Len[0]);
        MemBWCtrl_Resp_queue[0].push_back(pkt);
    } else {
        return false;
    }
    if (cpuPort.getblockedPacket() == nullptr && !respondEvent.scheduled())
        schedule(respondEvent, curTick());
    //////////////////////////////////////////////////////////////////////////////
    return true;
}

void MemBandwidthCtrl::sendResponse(PacketPtr pkt)
{
    DPRINTF(MemBandwidthCtrl, "Sending resp for addr %#x\n", pkt->getAddr());

    // The packet is now done. We're about to put it in the port, no need for
    // this object to continue to stall.
    // We need to free the resource before sending the packet in case the CPU
    // tries to send another request immediately (e.g., in the same callchain).

    // Simply forward to the memory port
    cpuPort.sendPacket(pkt);

    //if we are able to get more infomation trysendretry
    cpuPort.trySendRetry();
    //memPort.trySendRetry();
}

Tick 
MemBandwidthCtrl::handleAtomic(PacketPtr pkt){
//TODO handle immediately and return latency

//////////////////////////////////////////////////////////////////////////////
    return memPort.sendAtomic(pkt);

//////////////////////////////////////////////////////////////////////////////
}

void
MemBandwidthCtrl::handleFunctional(PacketPtr pkt)
{
    //TODO handle functional need to response immediately
    //////////////////////////////////////////////////////////////////////////////
    memPort.sendFunctional(pkt);
    //////////////////////////////////////////////////////////////////////////////
}

void
MemBandwidthCtrl::processNextReqEvent()
{    
    //TODO
    //printf("%d\n", Cur_Len[0]);
    if (Partition_Mode == BW_RW_Split) {
        DPRINTF(MemBWCtrlQueue, "Read queue Len %d\n", Read_Len[0]);
        DPRINTF(MemBWCtrlQueue, "Write queue Len %d\n", Write_Len[0]);
        Last_Window = 0;
        bool canIssue = true;
        if (BWCtrl_State == BWCTRL_READ_STATE && Read_Len[0] > 0) {
            PacketPtr Readpkt = MemBWCtrl_Read_queue[0][0];
            DPRINTF(MemBandwidthCtrl, "%s\n", Readpkt);
            Addr Raddr = Readpkt->getAddr();
            Addr Rsize = Readpkt->getSize();
            //DPRINTF(MemBandwidthCtrl, "%s\n", Readpkt);
            for (const auto& Writepkt : MemBWCtrl_Write_queue[0]) {
                Addr Waddr = Writepkt->getAddr();
                Addr Wsize = Writepkt->getSize();
                if ((Waddr <= Raddr && Raddr < Waddr + Wsize) || (Waddr < Raddr + Rsize && Raddr + Rsize <= Waddr + Wsize)) {
                    canIssue = false;
                    break;
                }
            }
            if (canIssue) {
                DPRINTF(MemBandwidthCtrl, "packet: %s\n", Readpkt->print());
                if (memPort.sendPacket(Readpkt)) {
                        Read_Len[0]--;
                    for (int i = 0; i < Read_Len[0]; i++)
                        MemBWCtrl_Read_queue[0][i] = MemBWCtrl_Read_queue[0][i + 1];
                    MemBWCtrl_Read_queue[0].pop_back();
                    cpuPort.trySendRetry();

                    if ((Read_Len[0] || Write_Len[0]) && !nextReqEvent.scheduled()) {
                        schedule(nextReqEvent, curTick());
                    }
                    return;
                }
            }else {
                BWCtrl_State = BWCTRL_WRITE_STATE;
            }
        }
        if (BWCtrl_State == BWCTRL_WRITE_STATE) {
            if (Write_Len[0] > 0) {
                PacketPtr pkt = MemBWCtrl_Write_queue[0][0];
                DPRINTF(MemBandwidthCtrl, "packet: %s\n", pkt->print());
                if (memPort.sendPacket(pkt)) {
                    Write_Len[0]--;
                    for (int i = 0; i < Write_Len[0]; i++)
                        MemBWCtrl_Write_queue[0][i] = MemBWCtrl_Write_queue[0][i + 1];
                    MemBWCtrl_Write_queue[0].pop_back();
                    cpuPort.trySendRetry();
                    if (Write_Len[0] == 0)
                        BWCtrl_State = BWCTRL_READ_STATE; 
                    if ((Read_Len[0] || Write_Len[0]) && !nextReqEvent.scheduled()) {
                        schedule(nextReqEvent, curTick());
                    }
                }
            }
        }
    }

    if (Partition_Mode == BW_Default) {
        DPRINTF(MemBWCtrlQueue, "Req queue Len %d\n", Cur_Len[0]);
        Last_Window = 0;
        PacketPtr pkt = MemBWCtrl_Req_queue[0][0];
        DPRINTF(MemBandwidthCtrl, "packet: %s\n", pkt->print());
        if (memPort.sendPacket(pkt)) {
            Cur_Len[0]--;
            for (int i = 0; i < Cur_Len[0]; i++)
                MemBWCtrl_Req_queue[0][i] = MemBWCtrl_Req_queue[0][i + 1];
            MemBWCtrl_Req_queue[0].pop_back();
            //////////////////////////////////////////////////////////////////////////////
            //responce to cpu side port
            //sendResponse(pkt);
            //send req to mem side port
            
            cpuPort.trySendRetry();
            //////////////////////////////////////////////////////////////////////////////
            if (Cur_Len[0] && !nextReqEvent.scheduled()) {
                schedule(nextReqEvent, curTick());
            }
        }
    }

    if (Partition_Mode == BW_Priority) {
        Last_Window = 0;
        int pos = 0;
        while (Cur_Len[pos] == 0 && pos < QoS_Type)
            pos++;
        //printf("Pos %d\n", pos);
        PacketPtr pkt = MemBWCtrl_Req_queue[pos][0];
        DPRINTF(MemBandwidthCtrl, "packet: %s\n", pkt->print());
        if (memPort.sendPacket(pkt)) {
            Cur_Len[pos]--;
            for (int i = 0; i < Cur_Len[pos]; i++)
                MemBWCtrl_Req_queue[pos][i] = MemBWCtrl_Req_queue[pos][i + 1];
            MemBWCtrl_Req_queue[pos].pop_back();
        //////////////////////////////////////////////////////////////////////////////
        //responce to cpu side port
        //sendResponse(pkt);
        //send req to mem side port
            cpuPort.trySendRetry();
        //////////////////////////////////////////////////////////////////////////////
            pos = 0;
            while (Cur_Len[pos] == 0 && pos < QoS_Type)
                pos++;
            if (pos < QoS_Type && !nextReqEvent.scheduled()) {
                schedule(nextReqEvent, curTick());
            }
        }
    }

    if (Partition_Mode == BW_Portion) {
        Last_Window = 0;
        Tick t = curTick() / 1000 % 100;
        int ptid = PartID_with_Tick[t]; //ptid for partid
        int mem = ptid;
        while (Cur_TDM_Len[ptid] == 0 && (ptid + 1) % PARTIDNUM != mem)
            ptid = (ptid + 1) % PARTIDNUM;
        PacketPtr pkt = MemBWCtrl_Req_TDM_queue[ptid][0];
        DPRINTF(MemBandwidthCtrl, "packet: %s\n", pkt->print());
        if (memPort.sendPacket(pkt)) {
            Cur_TDM_Len[ptid]--;
            for (int i = 0; i < Cur_TDM_Len[ptid]; i++)
                MemBWCtrl_Req_TDM_queue[ptid][i] = MemBWCtrl_Req_TDM_queue[ptid][i + 1];
            MemBWCtrl_Req_TDM_queue[ptid].pop_back();
            //////////////////////////////////////////////////////////////////////////////
            //responce to cpu side port
            //sendResponse(pkt);
            //send req to mem side port

            cpuPort.trySendRetry();
            //////////////////////////////////////////////////////////////////////////////
            ptid = 0;
            while (Cur_TDM_Len[ptid] == 0 && ptid < PARTIDNUM)
                ptid++;
            if (ptid < PARTIDNUM && !nextReqEvent.scheduled()) {
                schedule(nextReqEvent, curTick());
            }
        }
    }

    if (Partition_Mode == BW_MINMAX) {
        Tick now = curTick();
        if (Last_Window == 0)
            Last_Window = now;
        if (now - Last_Window >= Mem_Bandwidth_Monitor_Window) {
            Last_Window = now;
            for (int i = 0; i < PARTIDNUM; i++) {
                double MBP = 1.0 * MonBandwidth[i] / (1.0 * Packet_cnt);
                if (MBP > MBMAX[i])
                    PRI[i] = PRI_L;
                if (MBP > MBMIN[i] && MBP <= MBMAX[i])
                    PRI[i] = PRI_M;
                if (MBP > MBMIN[i] / 2 && MBP <= MBMIN[i])
                    PRI[i] = PRI_H;
                if (MBP<= MBMIN[i] / 2)
                    PRI[i] = PRI_HH;
            }
        }

        int pos = 0;
        while (Cur_Len[pos] == 0 && pos < QoS_Type)
            pos++;
        PacketPtr pkt = MemBWCtrl_Req_queue[pos][0];
        DPRINTF(MemBandwidthCtrl, "packet: %s\n", pkt->print());
        if (memPort.sendPacket(pkt)) {
            Cur_Len[pos]--;
            for (int i = 0; i < Cur_Len[pos]; i++)
                MemBWCtrl_Req_queue[pos][i] = MemBWCtrl_Req_queue[pos][i + 1];
            MemBWCtrl_Req_queue[pos].pop_back();
            //////////////////////////////////////////////////////////////////////////////
            //responce to cpu side port
            //sendResponse(pkt);
            //send req to mem side port
            cpuPort.trySendRetry();
            int partid = pkt->requestorId();
            Packet_cnt++;
            MonBandwidth[partid]++;
            //////////////////////////////////////////////////////////////////////////////
            pos = 0;
            while (Cur_Len[pos] == 0 && pos < QoS_Type)
                pos++;
            if (pos < QoS_Type && !nextReqEvent.scheduled()) {
                schedule(nextReqEvent, curTick());
            }
        }
    }
}

void
MemBandwidthCtrl::processRespondEvent()
{
    PacketPtr pkt = MemBWCtrl_Resp_queue[0][0];
    DPRINTF(MemBandwidthCtrl, "packet: %s\n", pkt->print());
    if (cpuPort.sendPacket(pkt)) {
        Resp_Len[0]--;
        for (int i = 0; i < Resp_Len[0]; i++)
            MemBWCtrl_Resp_queue[0][i] = MemBWCtrl_Resp_queue[0][i + 1];
        MemBWCtrl_Resp_queue[0].pop_back();
            //////////////////////////////////////////////////////////////////////////////
            //responce to cpu side port
            //sendResponse(pkt);
            //send req to mem side port

        memPort.trySendRetry();
            //////////////////////////////////////////////////////////////////////////////
        if (Resp_Len[0] && !respondEvent.scheduled()) {
            schedule(respondEvent, curTick());
        }
    }
}

AddrRangeList
MemBandwidthCtrl::getAddrRanges() const
{
    DPRINTF(MemBandwidthCtrl, "Sending new ranges\n");
    // Just use the same ranges as whatever is on the memory side.
    return memPort.getAddrRanges();
}

void
MemBandwidthCtrl::sendRangeChange() const
{
    cpuPort.sendRangeChange();
}

MemBandwidthCtrl::MemBandwidthCtrlStats::MemBandwidthCtrlStats(statistics::Group *parent)
      : statistics::Group(parent)
{
    
}

} // namespace gem5
