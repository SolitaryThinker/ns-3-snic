#include "flow.h"

namespace ns3
{

FlowId::FlowId(Ipv4Address srcIp,
               uint16_t srcPort,
               Ipv4Address dstIp,
               uint16_t dstPort,
               uint16_t protocol)
    : m_srcIp(srcIp),
      m_dstIp(dstIp),
      m_srcPort(srcPort),
      m_dstPort(dstPort),
      m_protocol(protocol)
{
}

FlowId::FlowId(const SnicSchedulerHeader& snicHeader)
{
    FlowId(snicHeader.GetSourceIp(),
           snicHeader.GetSourcePort(),
           snicHeader.GetDestinationIp(),
           snicHeader.GetDestinationPort(),
           snicHeader.GetProtocol());
}

// FlowId::FlowId(const SnicHeader& snicHeader)
//{
// FlowId(snicHeader.GetSourceIp(),
// snicHeader.GetSourcePort(),
// snicHeader.GetDestinationIp(),
// snicHeader.GetDestinationPort(),
// snicHeader.GetProtocol());
//}

} // namespace ns3
