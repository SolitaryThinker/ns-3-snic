#ifndef SNIC_FLOW_H
#define SNIC_FLOW_H

#include "ns3/ipv4-address.h"
#include "ns3/snic-header.h"
#include "ns3/snic-scheduler-header.h"

namespace ns3
{

class FlowId
{
  public:
    FlowId(Ipv4Address srcIp,
           uint16_t srcPort,
           Ipv4Address dstIp,
           uint16_t dstPort,
           uint16_t protocol);
    FlowId(const SnicSchedulerHeader& snicHeader);

  private:
    friend bool operator==(const FlowId& a, const FlowId& b);
    friend bool operator!=(const FlowId& a, const FlowId& b);
    friend bool operator<(const FlowId& a, const FlowId& b);
    Ipv4Address m_srcIp;
    Ipv4Address m_dstIp;
    uint16_t m_srcPort;
    uint16_t m_dstPort;
    uint16_t m_protocol;
};

inline bool
operator==(const FlowId& a, const FlowId& b)
{
    return (a.m_srcIp == b.m_srcIp && a.m_dstIp == b.m_dstIp && a.m_srcPort == b.m_srcPort &&
            a.m_dstPort == b.m_dstPort && a.m_protocol == b.m_protocol);
}

inline bool
operator!=(const FlowId& a, const FlowId& b)
{
    return !(a == b);
}

inline bool
operator<(const FlowId& a, const FlowId& b)
{
    return (a.m_srcIp < b.m_srcIp);
}

} // namespace ns3

#endif // SNIC_FLOW_H
