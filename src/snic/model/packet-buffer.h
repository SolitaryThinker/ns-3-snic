#ifndef SNIC_PACKET_BUFFER_H
#define SNIC_PACKET_BUFFER_H

#include "ns3/ipv4-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/snic-header.h"

#include <map>
#include <stdint.h>

namespace ns3
{

class PacketBuffer : public Object
{
    class SnicHeader;

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
    void HandleWaitReplyTimeout;

    Entry* Add(FlowId flowId);

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
        PacketBuffer m_state;
        std::list<Ptr<Packet>> m_pending; //!< list of pending packets for the entry's IP
        uint32_t m_retries;               //!< rerty counter
    };

    class FlowId
    {
      public:
        FlowId(Address srcIp, uint16_t srcPort, Address dstIp, uint16_t dstPort, uint16_t protocol);
        FlowId(const SnicHeader snicHeader);

      private:
        friend bool operator==(const FlowId& a, const FlowId* b);
        friend bool operator!=(const FlowId& a, const FlowId& b);
        Address m_srcIp;
        Address m_dstIp;
        uint16_t m_srcPort;
        uint16_t m_dstPort;
        uint16_t m_protocol;
    };

    typedef std::map<FlowId, PacketBuffer::Entry*> Cache;
    typedef std::map<FlowId, PacketBuffer::Entry*>::iterator CacheI;

    void DoDispose() override;
    Cache m_packetBuffer; //!< the packet cache
};

inline bool
operator==(const PacketBuffer::FlowId& a, const PacketBuffer::FlowId& b)
{
    return (a.m_srcIp == b.m_srcIp && a.m_dstIp == b.m_dstIp && a.m_srcPort == b.m_srcPort &&
            a.m_dstPort == b.m_dstPort && a.m_protocol == b.m_protocl);
}

inline bool
operator!=(const PacketBuffer::FlowId& a, const PacketBuffer::FlowId& b)
{
    return !(a == b);
}

} // namespace ns3

#endif // SNIC_PACKET_BUFFER_H
