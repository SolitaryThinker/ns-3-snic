#ifndef SNIC_PACKET_BUFFER_H
#define SNIC_PACKET_BUFFER_H

#include "ns3/callback.h"
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
    class FlowId;

    PacketBuffer();
    ~PacketBuffer() override;

    void SetDevice(Ptr<NetDevice> device);
    Ptr<NetDevice> GetDevice() const;

    void SetWaitReplyTimeout(Time waitReplyTimeout);
    Time GetWaitReplyTimeout() const;
    void StartWaitReplyTimer();

    Entry* Add(const FlowId& flowId);

    // typedef std::pair<Ptr<Packet>, Ipv4Header> Ipv4PayloadHeaderPair;

    class Entry
    {
      public:
        Entry(PacketBuffer* packetBuffer);
        void MarkWaitReply(Ptr<const Packet>);
        bool IsDone() const;
        bool IsActive() const;
        bool IsWaitReply() const;
        bool IsExpired() const;

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
        std::list<Ptr<const Packet>> m_pending; //!< list of pending packets for the entry's IP
        uint32_t m_retries;               //!< rerty counter
    };

    class FlowId
    {
      public:
        FlowId(Address srcIp, uint16_t srcPort, Address dstIp, uint16_t dstPort, uint16_t protocol);
        FlowId(const SnicHeader& snicHeader);

      private:
        friend bool operator==(const FlowId& a, const FlowId& b);
        friend bool operator!=(const FlowId& a, const FlowId& b);
        friend bool operator<(const FlowId& a, const FlowId& b);
        Address m_srcIp;
        Address m_dstIp;
        uint16_t m_srcPort;
        uint16_t m_dstPort;
        uint16_t m_protocol;
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

inline bool
operator==(const PacketBuffer::FlowId& a, const PacketBuffer::FlowId& b)
{
    return (a.m_srcIp == b.m_srcIp && a.m_dstIp == b.m_dstIp && a.m_srcPort == b.m_srcPort &&
            a.m_dstPort == b.m_dstPort && a.m_protocol == b.m_protocol);
}

inline bool
operator!=(const PacketBuffer::FlowId& a, const PacketBuffer::FlowId& b)
{
    return !(a == b);
}


inline bool
operator<(const PacketBuffer::FlowId& a, const PacketBuffer::FlowId& b)
{
    return (a.m_srcIp < b.m_srcIp);
}

} // namespace ns3

#endif // SNIC_PACKET_BUFFER_H
