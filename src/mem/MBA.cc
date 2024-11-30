#include "mem/MBA.hh"

#include "base/compiler.hh"
#include "base/random.hh"
#include "debug/MBA.hh"
#include "sim/system.hh"

namespace gem5
{

MBA::MBA(const MBAParams &params) :
    ClockedObject(params),
    cpuPort(params.name + ".cpu_side",this),
    memPort(params.name + ".mem_side", this),
    Additional_Cycles(params.MBACycles),
    nextReqEvent([this]{ processNextReqEvent(); }, name()),
    respondEvent([this]{ processRespondEvent(); }, name()),
    stats(this)
{
    SendingPacket = nullptr;
    ResponsingPacket = nullptr;
}

Port &
MBA::getPort(const std::string &if_name, PortID idx)
{
    // This is the name from the Python SimObject declaration in MBA.py
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
MBA::CPUSidePort::sendPacket(PacketPtr pkt)
{
    // Note: This flow control is very simple since the cache is blocking.

    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    // If we can't send the packet across the port, store it for later.
    DPRINTF(MBA, "Sending %s to CPU\n", pkt->print());
    if (!sendTimingResp(pkt)) {
        DPRINTF(MBA, "failed!\n");
        blockedPacket = pkt;
        return false;
    }
    return true;
}

AddrRangeList
MBA::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

void
MBA::CPUSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) {
        // Only send a retry if the port is now completely free
        needRetry = false;
        DPRINTF(MBA, "Sending retry req.\n");
        sendRetryReq();
    }
}

Tick
MBA::CPUSidePort::recvAtomic(PacketPtr pkt) {
    //TODO atomic request return latency
    return owner->handleAtomic(pkt);
}

void
MBA::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    // Just forward to the cache.
    return owner->handleFunctional(pkt);
}

bool
MBA::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    DPRINTF(MBA, "Got request %s\n", pkt->print());
    if (needRetry) {
        // pay attention to dead block
        //blocked means peer can't do anymore
        //needretry means we can't deal it
        DPRINTF(MBA, "Request blocked\n");
        needRetry = true;
        return false;
    }
    // Just forward to the cache.
    if (!owner->handleRequest(pkt)) {
        DPRINTF(MBA, "Request failed\n");
        // stalling
        needRetry = true;
        return false;
    } else {
        DPRINTF(MBA, "Request succeeded\n");
        return true;
    }
}

void
MBA::CPUSidePort::recvTimingSnoopRsep(PacketPtr pkt) {
    DPRINTF(MBA, "Got Snoop Resp %s\n", pkt->print());
    owner->memPort.sendTimingSnoopResp(pkt);
}

void
MBA::CPUSidePort::recvRespRetry()
{
    // We should have a blocked packet if this function is called.
    assert(blockedPacket != nullptr);

    // Grab the blocked packet.
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    DPRINTF(MBA, "Retrying response pkt %s\n", pkt->print());
    // Try to resend it. It's possible that it fails again.
    //sendPacket(pkt);
    owner->schedule(owner->respondEvent, curTick());
    // We may now be able to accept new packets
    //trySendRetry();
}

bool
MBA::MemSidePort::sendPacket(PacketPtr pkt)
{
    // Note: This flow control is very simple since the cache is blocking.

    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    // If we can't send the packet across the port, store it for later.
    if (!sendTimingReq(pkt)) {
        //send failed block it until recvRetry
        blockedPacket = pkt;
        return false;
    }
    return true;
}

void
MBA::MemSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) {
        // Only send a retry if the port is now completely free
        needRetry = false;
        DPRINTF(MBA, "Sending retry req.\n");
        sendRetryResp();
    }
}

bool
MBA::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    DPRINTF(MBA, "Got response %s\n", pkt->print());
    if (needRetry) {
        // pay attention to dead block
        //blocked means peer can't do anymore
        //needretry means we can't deal it
        DPRINTF(MBA, "Response blocked\n");
        needRetry = true;
        return false;
    }
    // Just forward to the cache.
    if (!owner->handleResponse(pkt)) {
        DPRINTF(MBA, "Response failed\n");
        // stalling
        needRetry = true;
        return false;
    } else {
        DPRINTF(MBA, "Response succeeded\n");
        return true;
    }
}

void
MBA::MemSidePort::recvTimingSnoopReq(PacketPtr pkt) {
    DPRINTF(MBA, "Got Snoop Req %s\n", pkt->print());
    owner->cpuPort.sendTimingSnoopReq(pkt);
}

void
MBA::MemSidePort::recvReqRetry()
{
    // We should have a blocked packet if this function is called.
    assert(blockedPacket != nullptr);

    // Grab the blocked packet.
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    DPRINTF(MBA, "Retrying request pkt %s\n", pkt->print());

    // Try to resend it. It's possible that it fails again.
    owner->schedule(owner->nextReqEvent, curTick());
    //sendPacket(pkt);
}

void
MBA::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

bool
MBA::handleRequest(PacketPtr pkt)
{
    //if we can deal packet return true else return false
    DPRINTF(MBA, "Got request for addr %#x\n", pkt->getAddr());
    //TODO handle request

    if (SendingPacket == nullptr) {
        SendingPacket = pkt;
   } else {
        return false;
    }
    // Schedule an event after cache access latency to actually access
    if (memPort.getblockedPacket() == nullptr && !nextReqEvent.scheduled())
        schedule(nextReqEvent, curTick() + Additional_Cycles);

    return true;
}

bool
MBA::handleResponse(PacketPtr pkt)
{
    //if we can deal packet return true else return false
    DPRINTF(MBA, "Got response for addr %#x\n", pkt->getAddr());
    //TODO handle request
    if (ResponsingPacket == nullptr) {
        ResponsingPacket = pkt;
    } else {
        return false;
    }

    if (cpuPort.getblockedPacket() == nullptr && !respondEvent.scheduled())
        schedule(respondEvent, curTick());
    return true;
}

/*void MBA::sendResponse(PacketPtr pkt)
{
    DPRINTF(MBA, "Sending resp for addr %#x\n", pkt->getAddr());

    // The packet is now done. We're about to put it in the port, no need for
    // this object to continue to stall.
    // We need to free the resource before sending the packet in case the CPU
    // tries to send another request immediately (e.g., in the same callchain).

    // forward to the memory port
    cpuPort.sendPacket(pkt);

    //if we are able to get more infomation trysendretry
    cpuPort.trySendRetry();
    //memPort.trySendRetry();
}*/

Tick
MBA::handleAtomic(PacketPtr pkt){
//TODO handle immediately and return latency

//////////////////////////////////////////////////////////////////////////////
    return memPort.sendAtomic(pkt);

//////////////////////////////////////////////////////////////////////////////
}

void
MBA::handleFunctional(PacketPtr pkt)
{
    //TODO handle functional need to response immediately
    memPort.sendFunctional(pkt);
}

void
MBA::processNextReqEvent()
{
    PacketPtr pkt = SendingPacket;
    DPRINTF(MBA, "packet: %s\n", pkt->print());
    if (memPort.sendPacket(pkt)) {
        SendingPacket = nullptr;
        cpuPort.trySendRetry();
    }
}

void
MBA::processRespondEvent()
{
    PacketPtr pkt = ResponsingPacket;
    DPRINTF(MBA, "packet: %s\n", pkt->print());
    if (cpuPort.sendPacket(pkt)) {
        ResponsingPacket = nullptr;
        memPort.trySendRetry();
    }
}

AddrRangeList
MBA::getAddrRanges() const
{
    DPRINTF(MBA, "Sending new ranges\n");
    // Just use the same ranges as whatever is on the memory side.
    return memPort.getAddrRanges();
}

void
MBA::sendRangeChange() const
{
    cpuPort.sendRangeChange();
}

MBA::MBAStats::MBAStats(statistics::Group *parent)
      : statistics::Group(parent)
{
}

} // namespace gem5
