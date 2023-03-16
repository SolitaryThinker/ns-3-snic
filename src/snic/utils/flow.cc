#include "flow.h"

namespace ns3
{

// NS_LOG_COMPONENT_DEFINE("FlowId");
// NS_OBJECT_ENSURE_REGISTERED(FlowId);

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
    // NS_LOG_FUNCTION(this << flowId);
}

FlowId::FlowId(const SnicSchedulerHeader& snicHeader)
    : m_srcIp(snicHeader.GetSourceIp()),
      m_dstIp(snicHeader.GetDestinationIp()),
      m_srcPort(snicHeader.GetSourcePort()),
      m_dstPort(snicHeader.GetDestinationPort()),
      m_protocol(snicHeader.GetProtocol()),
      m_id(snicHeader.GetFlowId())
{
    // NS_LOG_FUNCTION(this);
}

FlowId::FlowId(const Ipv4Header& ipv4Header, const SnicHeader& snicHeader)
    : m_srcIp(ipv4Header.GetSource()),
      m_dstIp(ipv4Header.GetDestination()),
      m_srcPort(snicHeader.GetSourcePort()),
      m_dstPort(snicHeader.GetDestinationPort()),
      m_protocol(ipv4Header.GetProtocol()),
      m_id(snicHeader.GetFlowId())
{
    // NS_LOG_FUNCTION(this);
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
