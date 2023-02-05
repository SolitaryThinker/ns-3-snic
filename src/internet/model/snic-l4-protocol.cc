/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-l4-protocol.h"

#include "ipv4-end-point-demux.h"
#include "ipv4-end-point.h"
#include "ipv4-l3-protocol.h"
#include "ipv6-end-point-demux.h"
#include "ipv6-end-point.h"
#include "ipv6-l3-protocol.h"
#include "snic-socket-factory-impl.h"
#include "snic-socket-impl.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/object-vector.h"
#include "ns3/packet.h"
#include "ns3/snic-header.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicL4Protocol");

NS_OBJECT_ENSURE_REGISTERED(SnicL4Protocol);

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t SnicL4Protocol::PROT_NUMBER = 17;

TypeId
SnicL4Protocol::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SnicL4Protocol")
                            .SetParent<IpL4Protocol>()
                            .SetGroupName("Internet")
                            .AddConstructor<SnicL4Protocol>()
                            .AddAttribute("SocketList",
                                          "The list of sockets associated to this protocol.",
                                          ObjectVectorValue(),
                                          MakeObjectVectorAccessor(&SnicL4Protocol::m_sockets),
                                          MakeObjectVectorChecker<SnicSocketImpl>());
    return tid;
}

SnicL4Protocol::SnicL4Protocol()
    : m_endPoints(new Ipv4EndPointDemux()),
      m_endPoints6(new Ipv6EndPointDemux())
{
    NS_LOG_FUNCTION(this);
}

SnicL4Protocol::~SnicL4Protocol()
{
    NS_LOG_FUNCTION(this);
}

void
SnicL4Protocol::SetNode(Ptr<Node> node)
{
    m_node = node;
}

/*
 * This method is called by AggregateObject and completes the aggregation
 * by setting the node in the snic stack and link it to the ipv4 object
 * present in the node along with the socket factory
 */
void
SnicL4Protocol::NotifyNewAggregate()
{
    NS_LOG_FUNCTION(this);
    Ptr<Node> node = this->GetObject<Node>();
    Ptr<Ipv4> ipv4 = this->GetObject<Ipv4>();
    Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();

    if (!m_node)
    {
        if (node && (ipv4 || ipv6))
        {
            this->SetNode(node);
            Ptr<SnicSocketFactoryImpl> snicFactory = CreateObject<SnicSocketFactoryImpl>();
            snicFactory->SetSnic(this);
            node->AggregateObject(snicFactory);
        }
    }

    // We set at least one of our 2 down targets to the IPv4/IPv6 send
    // functions.  Since these functions have different prototypes, we
    // need to keep track of whether we are connected to an IPv4 or
    // IPv6 lower layer and call the appropriate one.

    if (ipv4 && m_downTarget.IsNull())
    {
        ipv4->Insert(this);
        this->SetDownTarget(MakeCallback(&Ipv4::Send, ipv4));
    }
    if (ipv6 && m_downTarget6.IsNull())
    {
        ipv6->Insert(this);
        this->SetDownTarget6(MakeCallback(&Ipv6::Send, ipv6));
    }
    IpL4Protocol::NotifyNewAggregate();
}

int
SnicL4Protocol::GetProtocolNumber() const
{
    return PROT_NUMBER;
}

void
SnicL4Protocol::DoDispose()
{
    NS_LOG_FUNCTION(this);
    for (std::vector<Ptr<SnicSocketImpl>>::iterator i = m_sockets.begin(); i != m_sockets.end();
         i++)
    {
        *i = nullptr;
    }
    m_sockets.clear();

    if (m_endPoints != nullptr)
    {
        delete m_endPoints;
        m_endPoints = nullptr;
    }
    if (m_endPoints6 != nullptr)
    {
        delete m_endPoints6;
        m_endPoints6 = nullptr;
    }
    m_node = nullptr;
    m_downTarget.Nullify();
    m_downTarget6.Nullify();
    /*
     = MakeNullCallback<void,Ptr<Packet>, Ipv4Address, Ipv4Address, uint8_t, Ptr<Ipv4Route> > ();
    */
    IpL4Protocol::DoDispose();
}

Ptr<Socket>
SnicL4Protocol::CreateSocket()
{
    NS_LOG_FUNCTION(this);
    Ptr<SnicSocketImpl> socket = CreateObject<SnicSocketImpl>();
    socket->SetNode(m_node);
    socket->SetSnic(this);
    m_sockets.push_back(socket);
    return socket;
}

Ipv4EndPoint*
SnicL4Protocol::Allocate()
{
    NS_LOG_FUNCTION(this);
    return m_endPoints->Allocate();
}

Ipv4EndPoint*
SnicL4Protocol::Allocate(Ipv4Address address)
{
    NS_LOG_FUNCTION(this << address);
    return m_endPoints->Allocate(address);
}

Ipv4EndPoint*
SnicL4Protocol::Allocate(Ptr<NetDevice> boundNetDevice, uint16_t port)
{
    NS_LOG_FUNCTION(this << boundNetDevice << port);
    return m_endPoints->Allocate(boundNetDevice, port);
}

Ipv4EndPoint*
SnicL4Protocol::Allocate(Ptr<NetDevice> boundNetDevice, Ipv4Address address, uint16_t port)
{
    NS_LOG_FUNCTION(this << boundNetDevice << address << port);
    return m_endPoints->Allocate(boundNetDevice, address, port);
}

Ipv4EndPoint*
SnicL4Protocol::Allocate(Ptr<NetDevice> boundNetDevice,
                         Ipv4Address localAddress,
                         uint16_t localPort,
                         Ipv4Address peerAddress,
                         uint16_t peerPort)
{
    NS_LOG_FUNCTION(this << boundNetDevice << localAddress << localPort << peerAddress << peerPort);
    return m_endPoints->Allocate(boundNetDevice, localAddress, localPort, peerAddress, peerPort);
}

void
SnicL4Protocol::DeAllocate(Ipv4EndPoint* endPoint)
{
    NS_LOG_FUNCTION(this << endPoint);
    m_endPoints->DeAllocate(endPoint);
}

Ipv6EndPoint*
SnicL4Protocol::Allocate6()
{
    NS_LOG_FUNCTION(this);
    return m_endPoints6->Allocate();
}

Ipv6EndPoint*
SnicL4Protocol::Allocate6(Ipv6Address address)
{
    NS_LOG_FUNCTION(this << address);
    return m_endPoints6->Allocate(address);
}

Ipv6EndPoint*
SnicL4Protocol::Allocate6(Ptr<NetDevice> boundNetDevice, uint16_t port)
{
    NS_LOG_FUNCTION(this << boundNetDevice << port);
    return m_endPoints6->Allocate(boundNetDevice, port);
}

Ipv6EndPoint*
SnicL4Protocol::Allocate6(Ptr<NetDevice> boundNetDevice, Ipv6Address address, uint16_t port)
{
    NS_LOG_FUNCTION(this << boundNetDevice << address << port);
    return m_endPoints6->Allocate(boundNetDevice, address, port);
}

Ipv6EndPoint*
SnicL4Protocol::Allocate6(Ptr<NetDevice> boundNetDevice,
                          Ipv6Address localAddress,
                          uint16_t localPort,
                          Ipv6Address peerAddress,
                          uint16_t peerPort)
{
    NS_LOG_FUNCTION(this << boundNetDevice << localAddress << localPort << peerAddress << peerPort);
    return m_endPoints6->Allocate(boundNetDevice, localAddress, localPort, peerAddress, peerPort);
}

void
SnicL4Protocol::DeAllocate(Ipv6EndPoint* endPoint)
{
    NS_LOG_FUNCTION(this << endPoint);
    m_endPoints6->DeAllocate(endPoint);
}

void
SnicL4Protocol::ReceiveIcmp(Ipv4Address icmpSource,
                            uint8_t icmpTtl,
                            uint8_t icmpType,
                            uint8_t icmpCode,
                            uint32_t icmpInfo,
                            Ipv4Address payloadSource,
                            Ipv4Address payloadDestination,
                            const uint8_t payload[8])
{
    NS_LOG_FUNCTION(this << icmpSource << icmpTtl << icmpType << icmpCode << icmpInfo
                         << payloadSource << payloadDestination);
    uint16_t src;
    uint16_t dst;
    src = payload[0] << 8;
    src |= payload[1];
    dst = payload[2] << 8;
    dst |= payload[3];

    Ipv4EndPoint* endPoint = m_endPoints->SimpleLookup(payloadSource, src, payloadDestination, dst);
    if (endPoint != nullptr)
    {
        endPoint->ForwardIcmp(icmpSource, icmpTtl, icmpType, icmpCode, icmpInfo);
    }
    else
    {
        NS_LOG_DEBUG("no endpoint found source=" << payloadSource
                                                 << ", destination=" << payloadDestination
                                                 << ", src=" << src << ", dst=" << dst);
    }
}

void
SnicL4Protocol::ReceiveIcmp(Ipv6Address icmpSource,
                            uint8_t icmpTtl,
                            uint8_t icmpType,
                            uint8_t icmpCode,
                            uint32_t icmpInfo,
                            Ipv6Address payloadSource,
                            Ipv6Address payloadDestination,
                            const uint8_t payload[8])
{
    NS_LOG_FUNCTION(this << icmpSource << icmpTtl << icmpType << icmpCode << icmpInfo
                         << payloadSource << payloadDestination);
    uint16_t src;
    uint16_t dst;
    src = payload[0] << 8;
    src |= payload[1];
    dst = payload[2] << 8;
    dst |= payload[3];

    Ipv6EndPoint* endPoint =
        m_endPoints6->SimpleLookup(payloadSource, src, payloadDestination, dst);
    if (endPoint != nullptr)
    {
        endPoint->ForwardIcmp(icmpSource, icmpTtl, icmpType, icmpCode, icmpInfo);
    }
    else
    {
        NS_LOG_DEBUG("no endpoint found source=" << payloadSource
                                                 << ", destination=" << payloadDestination
                                                 << ", src=" << src << ", dst=" << dst);
    }
}

enum IpL4Protocol::RxStatus
SnicL4Protocol::Receive(Ptr<Packet> packet, const Ipv4Header& header, Ptr<Ipv4Interface> interface)
{
    NS_LOG_FUNCTION(this << packet << header);
    SnicHeader snicHeader;
    if (Node::ChecksumEnabled())
    {
        snicHeader.EnableChecksums();
    }

    snicHeader.InitializeChecksum(header.GetSource(), header.GetDestination(), PROT_NUMBER);

    // We only peek at the header for now (instead of removing it) so that it will be intact
    // if we have to pass it to a IPv6 endpoint via:
    //
    //   SnicL4Protocol::Receive (Ptr<Packet> packet, Ipv6Address &src, Ipv6Address &dst, ...)

    packet->PeekHeader(snicHeader);

    if (!snicHeader.IsChecksumOk())
    {
        NS_LOG_INFO("Bad checksum : dropping packet!");
        return IpL4Protocol::RX_CSUM_FAILED;
    }

    NS_LOG_DEBUG("Looking up dst " << header.GetDestination() << " port "
                                   << snicHeader.GetDestinationPort());
    Ipv4EndPointDemux::EndPoints endPoints = m_endPoints->Lookup(header.GetDestination(),
                                                                 snicHeader.GetDestinationPort(),
                                                                 header.GetSource(),
                                                                 snicHeader.GetSourcePort(),
                                                                 interface);
    if (endPoints.empty())
    {
        if (this->GetObject<Ipv6L3Protocol>())
        {
            NS_LOG_LOGIC("  No Ipv4 endpoints matched on SnicL4Protocol, trying Ipv6 " << this);
            Ptr<Ipv6Interface> fakeInterface;
            Ipv6Header ipv6Header;
            Ipv6Address src = Ipv6Address::MakeIpv4MappedAddress(header.GetSource());
            Ipv6Address dst = Ipv6Address::MakeIpv4MappedAddress(header.GetDestination());
            ipv6Header.SetSource(src);
            ipv6Header.SetDestination(dst);
            return (this->Receive(packet, ipv6Header, fakeInterface));
        }

        NS_LOG_LOGIC("RX_ENDPOINT_UNREACH");
        return IpL4Protocol::RX_ENDPOINT_UNREACH;
    }

    packet->RemoveHeader(snicHeader);
    for (Ipv4EndPointDemux::EndPointsI endPoint = endPoints.begin(); endPoint != endPoints.end();
         endPoint++)
    {
        (*endPoint)->ForwardUp(packet->Copy(), header, snicHeader.GetSourcePort(), interface);
    }
    return IpL4Protocol::RX_OK;
}

enum IpL4Protocol::RxStatus
SnicL4Protocol::Receive(Ptr<Packet> packet, const Ipv6Header& header, Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << packet << header.GetSource() << header.GetDestination());
    SnicHeader snicHeader;
    if (Node::ChecksumEnabled())
    {
        snicHeader.EnableChecksums();
    }

    snicHeader.InitializeChecksum(header.GetSource(), header.GetDestination(), PROT_NUMBER);

    packet->RemoveHeader(snicHeader);

    if (!snicHeader.IsChecksumOk() && !header.GetSource().IsIpv4MappedAddress())
    {
        NS_LOG_INFO("Bad checksum : dropping packet!");
        return IpL4Protocol::RX_CSUM_FAILED;
    }

    NS_LOG_DEBUG("Looking up dst " << header.GetDestination() << " port "
                                   << snicHeader.GetDestinationPort());
    Ipv6EndPointDemux::EndPoints endPoints = m_endPoints6->Lookup(header.GetDestination(),
                                                                  snicHeader.GetDestinationPort(),
                                                                  header.GetSource(),
                                                                  snicHeader.GetSourcePort(),
                                                                  interface);
    if (endPoints.empty())
    {
        NS_LOG_LOGIC("RX_ENDPOINT_UNREACH");
        return IpL4Protocol::RX_ENDPOINT_UNREACH;
    }
    for (Ipv6EndPointDemux::EndPointsI endPoint = endPoints.begin(); endPoint != endPoints.end();
         endPoint++)
    {
        (*endPoint)->ForwardUp(packet->Copy(), header, snicHeader.GetSourcePort(), interface);
    }
    return IpL4Protocol::RX_OK;
}

void
SnicL4Protocol::Send(Ptr<Packet> packet,
                     Ipv4Address saddr,
                     Ipv4Address daddr,
                     uint16_t sport,
                     uint16_t dport)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << sport << dport);

    SnicHeader snicHeader;
    if (Node::ChecksumEnabled())
    {
        snicHeader.EnableChecksums();
        snicHeader.InitializeChecksum(saddr, daddr, PROT_NUMBER);
    }
    snicHeader.SetDestinationPort(dport);
    snicHeader.SetSourcePort(sport);

    packet->AddHeader(snicHeader);

    m_downTarget(packet, saddr, daddr, PROT_NUMBER, nullptr);
}

void
SnicL4Protocol::Send(Ptr<Packet> packet,
                     Ipv4Address saddr,
                     Ipv4Address daddr,
                     uint16_t sport,
                     uint16_t dport,
                     Ptr<Ipv4Route> route)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << sport << dport << route);

    SnicHeader snicHeader;
    if (Node::ChecksumEnabled())
    {
        snicHeader.EnableChecksums();
        snicHeader.InitializeChecksum(saddr, daddr, PROT_NUMBER);
    }
    snicHeader.SetDestinationPort(dport);
    snicHeader.SetSourcePort(sport);

    packet->AddHeader(snicHeader);

    m_downTarget(packet, saddr, daddr, PROT_NUMBER, route);
}

void
SnicL4Protocol::Send(Ptr<Packet> packet,
                     Ipv6Address saddr,
                     Ipv6Address daddr,
                     uint16_t sport,
                     uint16_t dport)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << sport << dport);

    SnicHeader snicHeader;
    if (Node::ChecksumEnabled())
    {
        snicHeader.EnableChecksums();
        snicHeader.InitializeChecksum(saddr, daddr, PROT_NUMBER);
    }
    snicHeader.SetDestinationPort(dport);
    snicHeader.SetSourcePort(sport);

    packet->AddHeader(snicHeader);

    m_downTarget6(packet, saddr, daddr, PROT_NUMBER, nullptr);
}

void
SnicL4Protocol::Send(Ptr<Packet> packet,
                     Ipv6Address saddr,
                     Ipv6Address daddr,
                     uint16_t sport,
                     uint16_t dport,
                     Ptr<Ipv6Route> route)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << sport << dport << route);

    SnicHeader snicHeader;
    if (Node::ChecksumEnabled())
    {
        snicHeader.EnableChecksums();
        snicHeader.InitializeChecksum(saddr, daddr, PROT_NUMBER);
    }
    snicHeader.SetDestinationPort(dport);
    snicHeader.SetSourcePort(sport);

    packet->AddHeader(snicHeader);

    m_downTarget6(packet, saddr, daddr, PROT_NUMBER, route);
}

void
SnicL4Protocol::SetDownTarget(IpL4Protocol::DownTargetCallback callback)
{
    NS_LOG_FUNCTION(this);
    m_downTarget = callback;
}

IpL4Protocol::DownTargetCallback
SnicL4Protocol::GetDownTarget() const
{
    return m_downTarget;
}

void
SnicL4Protocol::SetDownTarget6(IpL4Protocol::DownTargetCallback6 callback)
{
    NS_LOG_FUNCTION(this);
    m_downTarget6 = callback;
}

IpL4Protocol::DownTargetCallback6
SnicL4Protocol::GetDownTarget6() const
{
    return m_downTarget6;
}

} // namespace ns3
