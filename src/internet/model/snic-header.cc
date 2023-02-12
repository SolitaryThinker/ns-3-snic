/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-header.h"

#include "ns3/address-utils.h"

namespace ns3
{

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
      m_payloadSize(0),
      m_checksum(0),
      m_calcChecksum(false),
      m_goodChecksum(true)
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
SnicHeader::CopyPayload(uint8_t* buffer, size_t size)
{
    // check size is at least 8 bytes
    *((int64_t*)buffer) = m_payload;
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
    os << "snic_nt: " << m_nt << ", payload: " << m_payload
       << ", snic_length: " << m_payloadSize + GetSerializedSize() << " " << m_sourcePort << " > "
       << m_destinationPort;
}

uint32_t
SnicHeader::GetSerializedSize() const
{
    return 18;
}

void
SnicHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteHtonU16(m_sourcePort);
    i.WriteHtonU16(m_destinationPort);
    i.WriteHtonU16(m_nt);
    i.WriteHtonU64(m_payload);
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
