#include "flow.h"

namespace ns3
{

FlowId::FlowId(Ipv4Address srcIp,
               uint16_t srcPort,
               Ipv4Address dstIp,
               uint16_t dstPort,
               uint16_t protocol,
               uint64_t flowId)
    : m_srcIp(srcIp),
      m_dstIp(dstIp),
      m_srcPort(srcPort),
      m_dstPort(dstPort),
      m_protocol(protocol),
      m_id(flowId)
{
}

FlowId::FlowId(const SnicSchedulerHeader& snicHeader)
{
    FlowId(snicHeader.GetSourceIp(),
           snicHeader.GetSourcePort(),
           snicHeader.GetDestinationIp(),
           snicHeader.GetDestinationPort(),
           snicHeader.GetProtocol(),
           snicHeader.GetFlowId());
}

FlowId::FlowId(const Ipv4Header& ipv4Header, const SnicHeader& snicHeader)
{
    FlowId(ipv4Header.GetSource(),
           snicHeader.GetSourcePort(),
           ipv4Header.GetDestination(),
           snicHeader.GetDestinationPort(),
           ipv4Header.GetProtocol(),
           snicHeader.GetFlowId());
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
