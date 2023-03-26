/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-header.h"

#include "ns3/address-utils.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SnicRte);

SnicRte::SnicRte()
    : m_tag(0),
      m_prefix("127.0.0.1"),
      m_subnetMask("0.0.0.0"),
      m_nextHop("0.0.0.0"),
      m_metric(16)
{
}

TypeId
SnicRte::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SnicRte")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SnicRte>();
    return tid;
}

TypeId
SnicRte::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SnicRte::Print(std::ostream& os) const
{
    os << "prefix " << m_prefix << "/" << m_subnetMask.GetPrefixLength() << " Metric "
       << int(m_metric);
    os << " Tag " << int(m_tag) << " Next Hop " << m_nextHop;
}

uint32_t
SnicRte::GetSerializedSize() const
{
    return 20;
}

void
SnicRte::Serialize(Buffer::Iterator i) const
{
    i.WriteHtonU16(2);
    i.WriteHtonU16(m_tag);

    i.WriteHtonU32(m_prefix.Get());
    i.WriteHtonU32(m_subnetMask.Get());
    i.WriteHtonU32(m_nextHop.Get());
    i.WriteHtonU32(m_metric);
}

uint32_t
SnicRte::Deserialize(Buffer::Iterator i)
{
    uint16_t tmp;

    tmp = i.ReadNtohU16();
    if (tmp != 2)
    {
        return 0;
    }

    m_tag = i.ReadNtohU16();
    m_prefix.Set(i.ReadNtohU32());
    m_subnetMask.Set(i.ReadNtohU32());
    m_nextHop.Set(i.ReadNtohU32());

    m_metric = i.ReadNtohU32();

    return GetSerializedSize();
}

void
SnicRte::SetPrefix(Ipv4Address prefix)
{
    m_prefix = prefix;
}

Ipv4Address
SnicRte::GetPrefix() const
{
    return m_prefix;
}

void
SnicRte::SetSubnetMask(Ipv4Mask subnetMask)
{
    m_subnetMask = subnetMask;
}

Ipv4Mask
SnicRte::GetSubnetMask() const
{
    return m_subnetMask;
}

void
SnicRte::SetRouteTag(uint16_t routeTag)
{
    m_tag = routeTag;
}

uint16_t
SnicRte::GetRouteTag() const
{
    return m_tag;
}

void
SnicRte::SetRouteMetric(uint32_t routeMetric)
{
    m_metric = routeMetric;
}

uint32_t
SnicRte::GetRouteMetric() const
{
    return m_metric;
}

void
SnicRte::SetNextHop(Ipv4Address nextHop)
{
    m_nextHop = nextHop;
}

Ipv4Address
SnicRte::GetNextHop() const
{
    return m_nextHop;
}

std::ostream&
operator<<(std::ostream& os, const SnicRte& h)
{
    h.Print(os);
    return os;
}

NS_LOG_COMPONENT_DEFINE("SnicHeader");

NS_OBJECT_ENSURE_REGISTERED(SnicHeader);

/* The magic values below are used only for debugging.
 * They can be used to easily detect memory corruption
 * problems so you can see the patterns in memory.
 */
SnicHeader::SnicHeader()
    : m_sourcePort(0xfffd),
      m_destinationPort(0xfffd),
      m_nt(0),
      m_payload(-1),
      m_hasSeenNic(false),
      m_packetType(0),
      m_payloadSize(0),
      m_newFlow(false),
      m_checksum(0),
      m_calcChecksum(false),
      m_goodChecksum(true),
      m_delay(0)
{
}

SnicHeader::~SnicHeader()
{
    m_sourcePort = 0xfffe;
    m_destinationPort = 0xfffe;
    m_payloadSize = 0xfffe;
}

void
SnicHeader::AddNT(uint64_t nt)
{
    NS_LOG_FUNCTION(this << nt);
    m_nt = nt;
}

uint64_t
SnicHeader::GetNT()
{
    return m_nt;
}

void
SnicHeader::SetPayload(uint8_t* buffer, size_t size)
{
    NS_LOG_FUNCTION(this << *(int64_t*)buffer << size);
    // check size is at least 8 bytes
    m_payload = *((int64_t*)buffer);
}

void
SnicHeader::CopyPayload(uint8_t* buffer, size_t size) const
{
    // check size is at least 8 bytes
    *((int64_t*)buffer) = m_payload;
}

bool
SnicHeader::HasSeenNic() const
{
    return m_hasSeenNic;
}

void
SnicHeader::SetHasSeenNic()
{
    m_hasSeenNic = true;
}

bool
SnicHeader::IsNewFlow() const
{
    return m_newFlow;
}

void
SnicHeader::SetNewFlow(bool newFlow)
{
    m_hasSeenNic = newFlow;
}

uint16_t
SnicHeader::GetPacketType() const
{
    return m_packetType;
}

void
SnicHeader::SetPacketType(uint16_t packetType)
{
    m_packetType = packetType;
}

void
SnicHeader::SetFlowId(uint64_t flowId)
{
    m_flowId = flowId;
}

uint64_t
SnicHeader::GetFlowId() const
{
    return m_flowId;
}

void
SnicHeader::AddDelay(Time t)
{
    m_delay += t;
}

Time
SnicHeader::GetDelay() const
{
    return m_delay;
}

void
SnicHeader::EnableChecksums()
{
    m_calcChecksum = true;
}

void
SnicHeader::SetDestinationPort(uint16_t port)
{
    m_destinationPort = port;
}

void
SnicHeader::SetSourcePort(uint16_t port)
{
    m_sourcePort = port;
}

uint16_t
SnicHeader::GetSourcePort() const
{
    return m_sourcePort;
}

uint16_t
SnicHeader::GetDestinationPort() const
{
    return m_destinationPort;
}

void
SnicHeader::SetSourceIp(Address ip)
{
    m_source = ip;
}

void
SnicHeader::SetDestinationIp(Address ip)
{
    m_destination = ip;
}

Address
SnicHeader::GetSourceIp() const
{
    return m_source;
}

Address
SnicHeader::GetDestinationIp() const
{
    return m_destination;
}

void
SnicHeader::SetProtocol(uint8_t protocol)
{
    m_protocol = protocol;
}

uint8_t
SnicHeader::GetProtocol() const
{
    return m_protocol;
}

void
SnicHeader::InitializeChecksum(Address source, Address destination, uint8_t protocol)
{
    m_source = source;
    m_destination = destination;
    m_protocol = protocol;
}

void
SnicHeader::InitializeChecksum(Ipv4Address source, Ipv4Address destination, uint8_t protocol)
{
    m_source = source;
    m_destination = destination;
    m_protocol = protocol;
}

void
SnicHeader::InitializeChecksum(Ipv6Address source, Ipv6Address destination, uint8_t protocol)
{
    m_source = source;
    m_destination = destination;
    m_protocol = protocol;
}

uint16_t
SnicHeader::CalculateHeaderChecksum(uint16_t size) const
{
    Buffer buf = Buffer((2 * Address::MAX_SIZE) + 8);
    buf.AddAtStart((2 * Address::MAX_SIZE) + 8);
    Buffer::Iterator it = buf.Begin();
    uint32_t hdrSize = 0;

    WriteTo(it, m_source);
    WriteTo(it, m_destination);
    if (Ipv4Address::IsMatchingType(m_source))
    {
        it.WriteU8(0);           /* protocol */
        it.WriteU8(m_protocol);  /* protocol */
        it.WriteU8(size >> 8);   /* length */
        it.WriteU8(size & 0xff); /* length */
        hdrSize = 12;
    }
    else if (Ipv6Address::IsMatchingType(m_source))
    {
        it.WriteU16(0);
        it.WriteU8(size >> 8);   /* length */
        it.WriteU8(size & 0xff); /* length */
        it.WriteU16(0);
        it.WriteU8(0);
        it.WriteU8(m_protocol); /* protocol */
        hdrSize = 40;
    }

    it = buf.Begin();
    /* we don't CompleteChecksum ( ~ ) now */
    return ~(it.CalculateIpChecksum(hdrSize));
}

bool
SnicHeader::IsChecksumOk() const
{
    return m_goodChecksum;
}

void
SnicHeader::ForceChecksum(uint16_t checksum)
{
    m_checksum = checksum;
}

void
SnicHeader::ForcePayloadSize(uint16_t payloadSize)
{
    m_payloadSize = payloadSize;
}

TypeId
SnicHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SnicHeader")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SnicHeader>();
    return tid;
}

TypeId
SnicHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SnicHeader::Print(std::ostream& os) const
{
    os << "packetType: " << m_packetType << " seenSnic: " << m_hasSeenNic << " snic_nt: " << m_nt
       << ", payload: " << m_payload << ", snic_length: " << m_payloadSize + GetSerializedSize()
       << " " << m_sourcePort << " > " << m_destinationPort;
}

uint32_t
SnicHeader::GetSerializedSize() const
{
    return 30;
}

void
SnicHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteHtonU16(m_sourcePort);
    i.WriteHtonU16(m_destinationPort);
    i.WriteHtonU16(m_nt);
    i.WriteHtonU64(m_payload);
    i.WriteU8(m_hasSeenNic);
    i.WriteHtonU16(m_packetType);
    i.WriteU8(m_newFlow);
    i.WriteHtonU64(m_flowId);
    if (m_payloadSize == 0)
    {
        i.WriteHtonU16(start.GetSize());
    }
    else
    {
        i.WriteHtonU16(m_payloadSize);
    }

    if (m_checksum == 0)
    {
        i.WriteU16(0);

        if (m_calcChecksum)
        {
            uint16_t headerChecksum = CalculateHeaderChecksum(start.GetSize());
            i = start;
            uint16_t checksum = i.CalculateIpChecksum(start.GetSize(), headerChecksum);

            i = start;
            i.Next(6);
            i.WriteU16(checksum);
        }
    }
    else
    {
        i.WriteU16(m_checksum);
    }
}

uint32_t
SnicHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    m_sourcePort = i.ReadNtohU16();
    m_destinationPort = i.ReadNtohU16();
    m_nt = i.ReadNtohU16();
    m_payload = i.ReadNtohU64();
    m_hasSeenNic = i.ReadU8();
    m_packetType = i.ReadNtohU16();
    m_newFlow = i.ReadU8();
    m_flowId = i.ReadNtohU64();
    m_payloadSize = i.ReadNtohU16() - GetSerializedSize();
    m_checksum = i.ReadU16();

    if (m_calcChecksum)
    {
        uint16_t headerChecksum = CalculateHeaderChecksum(start.GetSize());
        i = start;
        uint16_t checksum = i.CalculateIpChecksum(start.GetSize(), headerChecksum);

        m_goodChecksum = (checksum == 0);
    }

    return GetSerializedSize();
}

uint16_t
SnicHeader::GetChecksum()
{
    return m_checksum;
}

} // namespace ns3
