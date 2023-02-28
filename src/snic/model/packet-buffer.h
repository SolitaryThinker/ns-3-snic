#ifndef SNIC_PACKET_BUFFER_H
#define SNIC_PACKET_BUFFER_H

#include "ns3/callback.h"
#include "ns3/flow.h"
#include "ns3/ipv4-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/snic-header.h"

#include <map>
#include <stdint.h>

namespace ns3
{
class SnicHeader;

class PacketBuffer : public Object
{
  public:
    static TypeId GetTypeId();

    class Entry;
    // class FlowId;

    PacketBuffer();
    ~PacketBuffer() override;

    void SetDevice(Ptr<NetDevice> device);
    Ptr<NetDevice> GetDevice() const;

    void SetWaitReplyTimeout(Time waitReplyTimeout);
    Time GetWaitReplyTimeout() const;
    void StartWaitReplyTimer();

    Entry* Add(const FlowId& flowId);

    Entry* Lookup(const FlowId& flowId);

    // typedef std::pair<Ptr<Packet>, Ipv4Header> Ipv4PayloadHeaderPair;

    class Entry
    {
      public:
        Entry(PacketBuffer* packetBuffer);
        void MarkActive(/* allocation */);
        void MarkWaitReply();
        void EnqueuePending(Ptr<Packet>);
        Ptr<Packet> DequeuePending();
        bool IsDone() const;
        bool IsActive() const;
        bool IsWaitReply() const;
        bool IsExpired() const;

        void SetIncomingPort(Ptr<NetDevice> incomingPort);
        void SetProtocol(uint16_t protocol);
        void SetSrc(const Address src);
        void SetDst(const Address dst);

        Ptr<NetDevice> GetIncomingPort() const;
        uint16_t GetProtocol() const;
        Address GetSrc() const;
        Address GetDst() const;

      private:
        enum PacketBufferEntryState_e
        {
            ACTIVE,
            WAIT_REPLY,
            DONE,
            EXPIRED
        };

        Time GetTimeout() const;

        // Time m_lastSeen; //!< last moment a packet from that address has been seen
        PacketBuffer* m_packetBuffer;
        PacketBufferEntryState_e m_state;
        std::list<Ptr<Packet>> m_pending; //!< list of pending packets for the entry's IP
                                          //
        Ptr<NetDevice> m_incomingPort;
        uint16_t m_protocol;
        Address m_src;
        Address m_dst;

        uint32_t m_retries;               //!< rerty counter
    };

    typedef std::map<FlowId, PacketBuffer::Entry*> Buffer;
    typedef std::map<FlowId, PacketBuffer::Entry*>::iterator BufferI;

    void DoDispose() override;
    Ptr<NetDevice> m_device;
    Time m_waitReplyTimeout; //!< cache reply state timeout
    EventId m_waitReplyTimer; //!< cache alive state timer
    Buffer m_packetBuffer; //!< the packet cache
    // Callback<void, Ptr<const ArpCache>, Ipv4Address>
    // m_arpRequestCallback; //!< reply timeout callback

    uint32_t m_maxRetries;    //!< max retries for a resolution
                              //
    void HandleWaitReplyTimeout();
    uint32_t m_pendingQueueSize; //!< number of packets waiting for a resolution
                                 //
};

} // namespace ns3

#endif // SNIC_PACKET_BUFFER_H
