#ifndef SNIC_FLOW_H
#define SNIC_FLOW_H

#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
//#include "ns3/object.h"
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
           uint16_t protocol,
           uint64_t id);
    FlowId(const SnicSchedulerHeader& snicHeader);
    FlowId(const Ipv4Header& ipv4Header, const SnicHeader& snicHeader);

    uint64_t GetId() const
    {
        return m_id;
    }

  private:
    friend bool operator==(const FlowId& a, const FlowId& b);
    friend bool operator!=(const FlowId& a, const FlowId& b);
    friend bool operator<(const FlowId& a, const FlowId& b);
    Ipv4Address m_srcIp;
    Ipv4Address m_dstIp;
    uint16_t m_srcPort;
    uint16_t m_dstPort;
    uint16_t m_protocol;
    uint64_t m_id;
};

inline bool
operator==(const FlowId& a, const FlowId& b)
{
    return (a.m_srcIp == b.m_srcIp && a.m_dstIp == b.m_dstIp && a.m_srcPort == b.m_srcPort &&
            a.m_dstPort == b.m_dstPort && a.m_protocol == b.m_protocol && a.m_id == b.m_id);
}

inline bool
operator!=(const FlowId& a, const FlowId& b)
{
    return !(a == b);
}

inline bool
operator<(const FlowId& a, const FlowId& b)
{
    // return (a.m_srcIp < b.m_srcIp);
    return (a.m_id < b.m_id);
    // return (a.m_srcIp < b.m_srcIp && a.m_dstIp < b.m_dstIp && a.m_srcPort < b.m_srcPort &&
    // a.m_dstPort < b.m_dstPort && a.m_protocol < b.m_protocol && a.m_id < b.m_id);
}

} // namespace ns3

#endif // SNIC_FLOW_H
