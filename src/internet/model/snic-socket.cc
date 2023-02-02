#include "snic-socket.h"

#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicSocket");

NS_OBJECT_ENSURE_REGISTERED(SnicSocket);

TypeId
SnicSocket::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SnicSocket")
            .SetParent<Socket>()
            .SetGroupName("Internet")
            .AddAttribute(
                "RcvBufSize",
                "SnicSocket maximum receive buffer size (bytes)",
                UintegerValue(131072),
                MakeUintegerAccessor(&SnicSocket::GetRcvBufSize, &SnicSocket::SetRcvBufSize),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("IpTtl",
                          "socket-specific TTL for unicast IP packets (if non-zero)",
                          UintegerValue(0),
                          MakeUintegerAccessor(&SnicSocket::GetIpTtl, &SnicSocket::SetIpTtl),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute("IpMulticastTtl",
                          "socket-specific TTL for multicast IP packets (if non-zero)",
                          UintegerValue(0),
                          MakeUintegerAccessor(&SnicSocket::GetIpMulticastTtl,
                                               &SnicSocket::SetIpMulticastTtl),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute(
                "IpMulticastIf",
                "interface index for outgoing multicast on this socket; -1 indicates to use "
                "default interface",
                IntegerValue(-1),
                MakeIntegerAccessor(&SnicSocket::GetIpMulticastIf, &SnicSocket::SetIpMulticastIf),
                MakeIntegerChecker<int32_t>())
            .AddAttribute("IpMulticastLoop",
                          "whether outgoing multicast sent also to loopback interface",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SnicSocket::GetIpMulticastLoop,
                                              &SnicSocket::SetIpMulticastLoop),
                          MakeBooleanChecker())
            .AddAttribute(
                "MtuDiscover",
                "If enabled, every outgoing ip packet will have the DF flag set.",
                BooleanValue(false),
                MakeBooleanAccessor(&SnicSocket::SetMtuDiscover, &SnicSocket::GetMtuDiscover),
                MakeBooleanChecker());
    return tid;
}

SnicSocket::SnicSocket()
{
    NS_LOG_FUNCTION(this);
}

SnicSocket::~SnicSocket()
{
    NS_LOG_FUNCTION(this);
}

} // namespace ns3
