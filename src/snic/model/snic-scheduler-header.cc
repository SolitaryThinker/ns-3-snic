/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-scheduler-header.h"

#include "ns3/address-utils.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicSchedulerHeader");

NS_OBJECT_ENSURE_REGISTERED(SnicSchedulerHeader);

/* The magic values below are used only for debugging.
 * They can be used to easily detect memory corruption
 * problems so you can see the patterns in memory.
 */
SnicSchedulerHeader::SnicSchedulerHeader()
    : m_bandwidthDemand(0),
      m_resourceDemand(0),
      m_sourcePort(0xfffd),
      m_destinationPort(0xfffd),
      m_packetType(0),
      m_protocol(0),
      m_flowId(0)
{
    NS_LOG_FUNCTION(this);
}

SnicSchedulerHeader::SnicSchedulerHeader(Ipv4Address srcIp,
                                         uint16_t srcPort,
                                         Ipv4Address dstIp,
                                         uint16_t dstPort,
                                         uint8_t protocol,
                                         uint64_t flowId)
    : m_bandwidthDemand(0),
      m_resourceDemand(0),
      m_sourcePort(srcPort),
      m_destinationPort(dstPort),
      m_packetType(0),
      m_source(srcIp),
      m_destination(dstIp),
      m_protocol(protocol),
      m_flowId(flowId)
{
    NS_LOG_FUNCTION(this << flowId);
    m_flowId = flowId;
    NS_LOG_FUNCTION(this << m_flowId);
}

SnicSchedulerHeader::SnicSchedulerHeader(Ipv4Header ipv4Header, SnicHeader snicHeader)
    : m_bandwidthDemand(0),
      m_resourceDemand(0),
      m_sourcePort(snicHeader.GetSourcePort()),
      m_destinationPort(snicHeader.GetDestinationPort()),
      m_packetType(0),
      m_source(ipv4Header.GetSource()),
      m_destination(ipv4Header.GetDestination()),
      m_protocol(ipv4Header.GetProtocol()),
      m_flowId(snicHeader.GetFlowId())
{
    NS_LOG_FUNCTION(this);
}

SnicSchedulerHeader::~SnicSchedulerHeader()
{
    m_sourcePort = 0xfffe;
    m_destinationPort = 0xfffe;
}

void
SnicSchedulerHeader::SetBandwidthDemand(double demand)
{
    m_bandwidthDemand = demand;
}

double
SnicSchedulerHeader::GetBandwidthDemand() const
{
    return m_bandwidthDemand;
}

void
SnicSchedulerHeader::SetResourceDemand(uint32_t demand)
{
    NS_LOG_FUNCTION(this << demand);
    m_resourceDemand = demand;
}

uint32_t
SnicSchedulerHeader::GetResourceDemand() const
{
    return m_resourceDemand;
}

void
SnicSchedulerHeader::SetNumNetworkTask(uint16_t num)
{
    m_numNetworkTask = num;
}

uint16_t
SnicSchedulerHeader::GetNumNetworkTask() const
{
    return m_numNetworkTask;
}

void
SnicSchedulerHeader::AddNT(uint64_t nt)
{
    NS_LOG_FUNCTION(this << nt);
    m_nt = nt;
}

uint64_t
SnicSchedulerHeader::GetNT()
{
    return m_nt;
}

uint16_t
SnicSchedulerHeader::GetPacketType() const
{
    return m_packetType;
}

void
SnicSchedulerHeader::SetPacketType(uint16_t packetType)
{
    m_packetType = packetType;
}

void
SnicSchedulerHeader::SetDestinationPort(uint16_t port)
{
    m_destinationPort = port;
}

void
SnicSchedulerHeader::SetSourcePort(uint16_t port)
{
    m_sourcePort = port;
}

uint16_t
SnicSchedulerHeader::GetSourcePort() const
{
    return m_sourcePort;
}

uint16_t
SnicSchedulerHeader::GetDestinationPort() const
{
    return m_destinationPort;
}

void
SnicSchedulerHeader::SetSourceIp(Ipv4Address ip)
{
    m_source = ip;
}

void
SnicSchedulerHeader::SetDestinationIp(Ipv4Address ip)
{
    m_destination = ip;
}

Ipv4Address
SnicSchedulerHeader::GetSourceIp() const
{
    return m_source;
}

Ipv4Address
SnicSchedulerHeader::GetDestinationIp() const
{
    return m_destination;
}

void
SnicSchedulerHeader::SetProtocol(uint8_t protocol)
{
    m_protocol = protocol;
}

uint8_t
SnicSchedulerHeader::GetProtocol() const
{
    return m_protocol;
}

void
SnicSchedulerHeader::SetFlowId(uint64_t flowId)
{
    m_flowId = flowId;
}

uint64_t
SnicSchedulerHeader::GetFlowId() const
{
    NS_LOG_FUNCTION(this << m_flowId);
    return m_flowId;
}

TypeId
SnicSchedulerHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SnicSchedulerHeader")
                            .SetParent<Header>()
                            .SetGroupName("Snic")
                            .AddConstructor<SnicSchedulerHeader>();
    return tid;
}

TypeId
SnicSchedulerHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SnicSchedulerHeader::Print(std::ostream& os) const
{
    os << "packetType: " << m_packetType;

    //<< " seenSnic: " << m_hasSeenNic << " snic_nt: " << m_nt
    //<< ", payload: " << m_payload << ", snic_length: " << m_payloadSize + GetSerializedSize()
    //<< " " << m_sourcePort << " > " << m_destinationPort;
}

uint32_t
SnicSchedulerHeader::GetSerializedSize() const
{
    return 66;
}

void
SnicSchedulerHeader::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION_NOARGS();
    Buffer::Iterator i = start;

    // i.WriteHtonU64(m_bandwidthDemand);
    i.Write((uint8_t*)&m_bandwidthDemand, 8);
    i.WriteHtonU32(m_resourceDemand);
    i.WriteHtonU16(m_numNetworkTask);
    i.WriteHtonU16(m_nt);

    i.WriteHtonU16(m_sourcePort);
    i.WriteHtonU16(m_destinationPort);
    i.WriteHtonU16(m_packetType);

    i.WriteHtonU32(m_source.Get());
    i.WriteHtonU32(m_destination.Get());
    i.WriteU8(m_protocol);
    i.WriteHtonU64(m_flowId);
}

uint32_t
SnicSchedulerHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION_NOARGS();
    Buffer::Iterator i = start;
    // m_bandwidthDemand = i.ReadNtohU64();
    i.Read((uint8_t*)&m_bandwidthDemand, 8);
    m_resourceDemand = i.ReadNtohU32();
    m_numNetworkTask = i.ReadNtohU16();
    m_nt = i.ReadNtohU16();

    m_sourcePort = i.ReadNtohU16();      //!< Source port
    m_destinationPort = i.ReadNtohU16(); //!< Destination port
    // anything other than 0 means this packet is internal to snic cluster
    m_packetType = i.ReadNtohU16();

    m_source.Set(i.ReadNtohU32()); //!< Source IP address
    m_destination.Set(i.ReadNtohU32()); //!< Destination IP address
    m_protocol = i.ReadU8();            //!< Protocol number
    m_flowId = i.ReadNtohU64();

    return GetSerializedSize();
}

} // namespace ns3
