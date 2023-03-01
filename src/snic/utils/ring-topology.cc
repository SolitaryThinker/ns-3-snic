#include "ring-topology.h"

#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/snic-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("RingTopologyHelper");

RingTopologyHelper::RingTopologyHelper(uint32_t nSnics, uint32_t nHosts, uint32_t schedulerIdx)
{
    NS_LOG_FUNCTION(this << nSnics << nHosts);

    // CsmaHelper csmaHelper;
    m_csmaHelper.SetChannelAttribute("DataRate", DataRateValue(5000000));
    m_csmaHelper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    SnicHelper snicHelper;
    // swtch.SetChannelAttribute("DataRate", DataRateValue(5000000));
    // swtch.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    for (uint32_t i = 0; i < nSnics; ++i)
    {
        NS_LOG_INFO("Creating sNIC cluster #" << i + 1);
        snicHelper
            .CreateSnic(m_snics, 1, m_terminals, m_csmaSwitches, m_terminalDevices, m_csmaHelper);
    }

    NetDeviceContainer snicLink;
    if (nSnics < 4)
    {
        // less than 4 snics in a ring topology is fully connected
        snicLink.Add(m_csmaHelper.Install(m_csmaSwitches));
    }
    else
    {
        // otherwise we need to manually connect two by two in a ring
        for (uint32_t i = 0; i < nSnics; ++i)
        {
            NS_LOG_INFO("Linking node " << i << " and node " << (i + 1) % nSnics);
            NodeContainer tmp;
            // add the current sNIC
            tmp.Add(m_csmaSwitches.Get(i));
            // add the next sNIC, rolling over if needed
            tmp.Add(m_csmaSwitches.Get((i + 1) % nSnics));
            // create an independent csma channel for these two
            snicLink.Add(m_csmaHelper.Install(tmp));
        }
    }

    for (uint32_t i = 0; i < nSnics; ++i)
    {
        snicHelper.AddPort(m_snics.Get(i), snicLink.Get(i));
    }

    // add TOR switch
    NodeContainer torSwitchNode;
    torSwitchNode.Create(1);
    // add scheduler nic

    // snicHelper.AddPort(m_snics.Get(1), snicLink.Get(1));
    // snicHelper.AddPort(m_snics.Get(2), snicLink.Get(2));
    // snicHelper.AddPort(m_snics.Get(3), snicLink.Get(3));

    // Add internet stack to the terminals
    SnicStackHelper internet;
    internet.Install(m_terminals);
    internet.Install(m_csmaSwitches);

    // We've got the "hardware" in place.  Now we need to add IP addresses.
    //
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    m_interfaces = ipv4.Assign(m_terminalDevices);
    // NS_LOG_INFO("m_snics helper: " << m_snics);
    m_snic_interfaces = ipv4.Assign(m_snics);

    // set scheduler
    for (NetDeviceContainer::Iterator i = m_snics.Begin(); i != m_snics.End(); ++i)
    {
        NS_LOG_LOGIC("snic_ptr " << *i);
        NS_LOG_LOGIC("addresses ");
        Ptr<SnicNetDevice> snic = DynamicCast<SnicNetDevice, NetDevice>(*i);
        snic->SetSchedulerAddress(m_snic_interfaces.GetAddress(schedulerIdx));
        snic->SetIpAddress(m_snic_interfaces.GetAddress(i - m_snics.Begin()));
    }
    DynamicCast<SnicNetDevice, NetDevice>(m_snics.Get(schedulerIdx))->SetIsScheduler(true);
}

RingTopologyHelper::~RingTopologyHelper()
{
    NS_LOG_FUNCTION_NOARGS();
}

NodeContainer
RingTopologyHelper::GetTerminals() const
{
    return m_terminals;
}

NodeContainer
RingTopologyHelper::GetCsmaSwitches() const
{
    return m_csmaSwitches;
}

NetDeviceContainer
RingTopologyHelper::GetTerminalDevices() const
{
    return m_terminalDevices;
}

NetDeviceContainer
RingTopologyHelper::GetSnics() const
{
    return m_snics;
}

Ipv4InterfaceContainer
RingTopologyHelper::GetInterfaces() const
{
    return m_interfaces;
}

Ipv4InterfaceContainer
RingTopologyHelper::GetSnicInterfaces() const
{
    return m_snic_interfaces;
}

CsmaHelper
RingTopologyHelper::GetCsmaHelper() const
{
    return m_csmaHelper;
}

} // namespace ns3
