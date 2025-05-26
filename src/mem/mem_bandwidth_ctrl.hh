#ifndef __MEM_BANDWIDTH_CTRL_HH__
#define __MEM_BANDWIDTH_CTRL_HH__

#include <unordered_map>
#include <map>

#include "mem/port.hh"
#include "sim/clocked_object.hh"
#include "params/MemBandwidthCtrl.hh"

#define Time_Slice 100
#define QoS_Type     4
#define BW_Default   0
#define BW_Priority  1
#define BW_MINMAX    2
#define BW_Portion   3
#define BW_RW_Split  4
#define BW_Hybrid    5
#define PRI_HH       0
#define PRI_H        1
#define PRI_M        2
#define PRI_L        3
#define PARTIDNUM   16

#define BWCTRL_READ_STATE   0
#define BWCTRL_WRITE_STATE  1

namespace gem5
{

typedef std::vector<PacketPtr> PacketQueue;

class MemBandwidthCtrl : public ClockedObject
{
  private:

    /**
     * Port on the CPU-side that receives requests.
     * Mostly just forwards requests to the cache (owner)
     */
    class CPUSidePort : public ResponsePort
    {
      private:

        /// The object that owns this object (SimpleCache)
        MemBandwidthCtrl *owner;

        /// True if the port needs to send a retry req.
        bool needRetry;

        /// If we tried to send a packet and it was blocked, store it here
        PacketPtr blockedPacket;

        bool Blocked;

      public:
        /**
         * Constructor. Just calls the superclass constructor.
         */
        CPUSidePort(const std::string& name, MemBandwidthCtrl *owner) :
            ResponsePort(name, owner), owner(owner), needRetry(false),
            blockedPacket(nullptr), Blocked(false)
        { }

        /**
         * Send a packet across this port. This is called by the owner and
         * all of the flow control is hanled in this function.
         * This is a convenience function for the SimpleCache to send pkts.
         *
         * @param packet to send.
         */
        bool sendPacket(PacketPtr pkt);
        
        PacketPtr getblockedPacket() {
            return blockedPacket;
        }

        /**
         * Get a list of the non-overlapping address ranges the owner is
         * responsible for. All response ports must override this function
         * and return a populated list with at least one item.
         *
         * @return a list of ranges responded to
         */
        AddrRangeList getAddrRanges() const override;

        /**
         * Send a retry to the peer port only if it is needed. This is called
         * from the SimpleCache whenever it is unblocked.
         */
        void trySendRetry();

      protected:
        /**
         * Receive an atomic request packet from the request port.
         * No need to implement in this simple cache.
         */
        Tick recvAtomic(PacketPtr pkt) override;

        /**
         * Receive a functional request packet from the request port.
         * Performs a "debug" access updating/reading the data in place.
         *
         * @param packet the requestor sent.
         */
        void recvFunctional(PacketPtr pkt) override;

        /**
         * Receive a timing request from the request port.
         *
         * @param the packet that the requestor sent
         * @return whether this object can consume to packet. If false, we
         *         will call sendRetry() when we can try to receive this
         *         request again.
         */
        bool recvTimingReq(PacketPtr pkt) override;

        /**
         * Called by the request port if sendTimingResp was called on this
         * response port (causing recvTimingResp to be called on the request
         * port) and was unsuccessful.
         */
        void recvRespRetry() override;
    };

    /**
     * Port on the memory-side that receives responses.
     * Mostly just forwards requests to the cache (owner)
     */
    class MemSidePort : public RequestPort
    {
      private:
        /// The object that owns this object (SimpleCache)
        MemBandwidthCtrl *owner;

        /// If we tried to send a packet and it was blocked, store it here
        PacketPtr blockedPacket;

        bool Blocked;

        bool needRetry;

        int count;
      public:
        /**
         * Constructor. Just calls the superclass constructor.
         */
        MemSidePort(const std::string& name, MemBandwidthCtrl *owner) :
            RequestPort(name, owner), owner(owner), blockedPacket(nullptr),
            Blocked(false), needRetry(false), count(0)
        { }

        /**
         * Send a packet across this port. This is called by the owner and
         * all of the flow control is hanled in this function.
         * This is a convenience function for the SimpleCache to send pkts.
         *
         * @param packet to send.
         */
        bool sendPacket(PacketPtr pkt);

        PacketPtr getblockedPacket() {
            return blockedPacket;
        }

        /**
         * Send a retry to the peer port only if it is needed. This is called
         * from the SimpleCache whenever it is unblocked.
         */
        void trySendRetry();

      protected:
        /**
         * Receive a timing response from the response port.
         */
        bool recvTimingResp(PacketPtr pkt) override;

        /**
         * Called by the response port if sendTimingReq was called on this
         * request port (causing recvTimingReq to be called on the response
         * port) and was unsuccesful.
         */
        void recvReqRetry() override;

        /**
         * Called to receive an address range change from the peer response
         * port. The default implementation ignores the change and does
         * nothing. Override this function in a derived class if the owner
         * needs to be aware of the address ranges, e.g. in an
         * interconnect component like a bus.
         */
        void recvRangeChange() override;
    };

    /**
     * Handle the request from the CPU side. Called from the CPU port
     * on a timing request.
     *
     * @param requesting packet
     * @param id of the port to send the response
     * @return true if we can handle the request this cycle, false if the
     *         requestor needs to retry later
     */
    bool handleRequest(PacketPtr pkt);

    /**
     * Handle the respone from the memory side. Called from the memory port
     * on a timing response.
     *
     * @param responding packet
     * @return true if we can handle the response this cycle, false if the
     *         responder needs to retry later
     */
    bool handleResponse(PacketPtr pkt);

    /**
     * Send the packet to the CPU side.
     * This function assumes the pkt is already a response packet and forwards
     * it to the correct port. This function also unblocks this object and
     * cleans up the whole request.
     *
     * @param the packet to send to the cpu side
     */
    void sendResponse(PacketPtr pkt);

    /**
     * Handle a packet functionally. Update the data on a write and get the
     * data on a read. Called from CPU port on a recv functional.
     *
     * @param packet to functionally handle
     */
    void handleFunctional(PacketPtr pkt);

    Tick handleAtomic(PacketPtr pkt);

    /**
     * Access the cache for a timing access. This is called after the cache
     * access latency has already elapsed.
     */
    //void accessTiming(PacketPtr pkt);

    /**
     * Return the address ranges this cache is responsible for. Just use the
     * same as the next upper level of the hierarchy.
     *
     * @return the address ranges this cache is responsible for
     */
    AddrRangeList getAddrRanges() const;

    /**
     * Tell the CPU side to ask for our memory ranges.
     */
    void sendRangeChange() const;

    /// Latency to check the cache. Number of cycles for both hit and miss
    const Cycles latency;

    /// Instantiation of the CPU-side port
    CPUSidePort cpuPort;

    /// Instantiation of the memory-side port
    MemSidePort memPort;

    //QoS queue length
    int QoS_Queue_Len_HH, QoS_Queue_Len_H, QoS_Queue_Len_M, QoS_Queue_Len_L;
    int QoS_Len[QoS_Type + 2];
    int Cur_Len[QoS_Type + 2];
    int Read_Len[QoS_Type + 2];
    int Write_Len[QoS_Type + 2];
    int Resp_Len[QoS_Type + 2];
    int Cur_TDM_Len[PARTIDNUM];
    int Write_Threshold;
    uint64_t Time_Threshold;
    uint64_t BandwidthCounter;
    Tick LastCountTime;
    int Partition_Mode;
    int Test_Mode;
    int TDM_Len_Max;
    int PartID_with_Tick[Time_Slice];

    int BWCtrl_State;

    int MonBandwidth[PARTIDNUM];
    double MBMAX[PARTIDNUM], MBMIN[PARTIDNUM];
    int PRI[PARTIDNUM], Packet_cnt;

    PacketQueue MemBWCtrl_Req_queue[QoS_Type + 2];
    PacketQueue MemBWCtrl_Read_queue[QoS_Type + 2];
    PacketQueue MemBWCtrl_Write_queue[QoS_Type + 2];
    PacketQueue MemBWCtrl_Resp_queue[QoS_Type + 2];
    PacketQueue MemBWCtrl_Req_TDM_queue[PARTIDNUM];
    PacketQueue MemBWCtrl_Resp_TDM_queue[PARTIDNUM];

    std::map<Addr, Tick> PacketTimer;

    int cnt1, cnt2;
    Tick DealTime[1000000], BackTime[1000000];
    Tick avgDeal, avgBack;

    Tick Mem_Bandwidth_Monitor_Window;
    Tick Last_Window;

    void processNextReqEvent();
    EventFunctionWrapper nextReqEvent;

    void processRespondEvent();
    EventFunctionWrapper respondEvent;

    /// Cache statistics
  protected:
    struct MemBandwidthCtrlStats : public statistics::Group
    {
        MemBandwidthCtrlStats(statistics::Group *parent);
    } stats;

  public:

    /** constructor
     */
    MemBandwidthCtrl(const MemBandwidthCtrlParams &params);

    /**
     * Get a port with a given name and index. This is used at
     * binding time and returns a reference to a protocol-agnostic
     * port.
     *
     * @param if_name Port name
     * @param idx Index in the case of a VectorPort
     *
     * @return A reference to the given port
     */
    Port &getPort(const std::string &if_name,
                  PortID idx=InvalidPortID) override;

};

} // namespace gem5

#endif // __LEARNING_GEM5_SIMPLE_CACHE_SIMPLE_CACHE_HH__
