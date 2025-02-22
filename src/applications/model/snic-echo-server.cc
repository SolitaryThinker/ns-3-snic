#include "snic-echo-server.h"

#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/snic-socket.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicEchoServerApplication");

NS_OBJECT_ENSURE_REGISTERED(SnicEchoServer);

TypeId
SnicEchoServer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SnicEchoServer")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<SnicEchoServer>()
            .AddAttribute("Port",
                          "Port on which we listen for incoming packets.",
                          UintegerValue(9),
                          MakeUintegerAccessor(&SnicEchoServer::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&SnicEchoServer::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&SnicEchoServer::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

SnicEchoServer::SnicEchoServer()
{
    NS_LOG_FUNCTION(this);
}

SnicEchoServer::~SnicEchoServer()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    m_socket6 = nullptr;
}

void
SnicEchoServer::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
SnicEchoServer::StartApplication()
{
    NS_LOG_FUNCTION(this);

    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::SnicSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (addressUtils::IsMulticast(m_local))
        {
            Ptr<SnicSocket> snicSocket = DynamicCast<SnicSocket>(m_socket);
            if (snicSocket)
            {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                snicSocket->MulticastJoinGroup(0, m_local);
            }
            else
            {
                NS_FATAL_ERROR("Error: Failed to join multicast group");
            }
        }
    }

    if (!m_socket6)
    {
        TypeId tid = TypeId::LookupByName("ns3::SnicSocketFactory");
        m_socket6 = Socket::CreateSocket(GetNode(), tid);
        Inet6SocketAddress local6 = Inet6SocketAddress(Ipv6Address::GetAny(), m_port);
        if (m_socket6->Bind(local6) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (addressUtils::IsMulticast(local6))
        {
            Ptr<SnicSocket> snicSocket = DynamicCast<SnicSocket>(m_socket6);
            if (snicSocket)
            {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                snicSocket->MulticastJoinGroup(0, local6);
            }
            else
            {
                NS_FATAL_ERROR("Error: Failed to join multicast group");
            }
        }
    }

    m_socket->SetRecvCallback(MakeCallback(&SnicEchoServer::HandleRead, this));
    m_socket6->SetRecvCallback(MakeCallback(&SnicEchoServer::HandleRead, this));
}

void
SnicEchoServer::StopApplication()
{
    NS_LOG_FUNCTION(this);

    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    if (m_socket6)
    {
        m_socket6->Close();
        m_socket6->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
SnicEchoServer::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server received "
                                   << packet->GetSize() << " bytes from "
                                   << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                                   << InetSocketAddress::ConvertFrom(from).GetPort());
            std::ostringstream coll;
            packet->Print(coll);
            NS_LOG_DEBUG("packet content is " << coll.str());
            uint8_t buffer[10];
            packet->CopyData((uint8_t*)&buffer, 5);
            NS_LOG_DEBUG("packet is " << buffer);
        }
        else if (Inet6SocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server received "
                                   << packet->GetSize() << " bytes from "
                                   << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
                                   << Inet6SocketAddress::ConvertFrom(from).GetPort());
        }

        packet->RemoveAllPacketTags();
        packet->RemoveAllByteTags();

        NS_LOG_LOGIC("Echoing packet");
        socket->SendTo(packet, 0, from);

        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server sent "
                                   << packet->GetSize() << " bytes to "
                                   << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                                   << InetSocketAddress::ConvertFrom(from).GetPort());
        }
        else if (Inet6SocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server sent "
                                   << packet->GetSize() << " bytes to "
                                   << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
                                   << Inet6SocketAddress::ConvertFrom(from).GetPort());
        }
    }
}

} // Namespace ns3
