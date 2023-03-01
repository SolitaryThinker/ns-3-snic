#ifndef SNIC_RING_TOPOLOGY_HELPER_H
#define SNIC_RING_TOPOLOGY_HELPER_H

#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

namespace ns3
{

class RingTopologyHelper
{
  public:
    RingTopologyHelper(uint32_t nSnics, uint32_t nHosts, uint32_t schedulerIdx);
    ~RingTopologyHelper();

    Ptr<Node> GetNode(uint32_t n);
    NodeContainer GetTerminals() const;
    NodeContainer GetCsmaSwitches() const;
    NetDeviceContainer GetTerminalDevices() const;
    NetDeviceContainer GetSnics() const;
    Ipv4InterfaceContainer GetInterfaces() const;
    Ipv4InterfaceContainer GetSnicInterfaces() const;
    CsmaHelper GetCsmaHelper() const;

  private:
    uint32_t m_size;
    // list of end host nodes
    NodeContainer m_terminals;
    // List of csma devices at each end host
    NetDeviceContainer m_terminalDevices;
    // List of sNICs
    NetDeviceContainer m_snics;
    // List of Nodes for switch
    NodeContainer m_csmaSwitches;

    Ipv4InterfaceContainer m_interfaces;
    Ipv4InterfaceContainer m_snic_interfaces;
    CsmaHelper m_csmaHelper;
};

} // namespace ns3

#endif // SNIC_RING_TOPOLOGY_HELPER_H
