/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-helper.h"

//#include "snic-channel.h"

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/snic-net-device.h"
#include "ns3/trace-helper.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicHelper");

SnicHelper::SnicHelper()
{
  m_deviceFactory.SetTypeId("ns3::SnicNetDevice");
  // m_channelFactory.SetTypeId("ns3::SnicChannel");
  m_enableFlowControl = true;
}

void
SnicHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
    m_deviceFactory.Set(n1, v1);
}

// void
// SnicHelper::SetChannelAttribute(std::string n1, const AttributeValue& v1)
//{
// m_channelFactory.Set(n1, v1);
//}

void
SnicHelper::DisableFlowControl()
{
    m_enableFlowControl = false;
}

/*
void
SnicHelper::EnablePcapInternal(std::string prefix,
                                       Ptr<NetDevice> nd,
                                       bool promiscuous,
                                       bool explicitFilename)
{
    //
    // All of the Pcap enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type SnicNetDevice.
    //
    Ptr<SnicNetDevice> device = nd->GetObject<SnicNetDevice>();
    if (!device)
    {
        NS_LOG_INFO("SnicHelper::EnablePcapInternal(): Device "
                    << device << " not of type ns3::SnicNetDevice");
        return;
    }

    PcapHelper pcapHelper;

    std::string filename;
    if (explicitFilename)
    {
        filename = prefix;
    }
    else
    {
        filename = pcapHelper.GetFilenameFromDevice(prefix, device);
    }

    Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_PPP);
    pcapHelper.HookDefaultSink<SnicNetDevice>(device, "PromiscSniffer", file);
}

void
SnicHelper::EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                                        std::string prefix,
                                        Ptr<NetDevice> nd,
                                        bool explicitFilename)
{
    //
    // All of the ascii enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type SnicNetDevice.
    //
    Ptr<SnicNetDevice> device = nd->GetObject<SnicNetDevice>();
    if (!device)
    {
        NS_LOG_INFO("SnicHelper::EnableAsciiInternal(): Device "
                    << device << " not of type ns3::SnicNetDevice");
        return;
    }

    //
    // Our default trace sinks are going to use packet printing, so we have to
    // make sure that is turned on.
    //
    Packet::EnablePrinting();

    //
    // If we are not provided an OutputStreamWrapper, we are expected to create
    // one using the usual trace filename conventions and do a Hook*WithoutContext
    // since there will be one file per context and therefore the context would
    // be redundant.
    //
    if (!stream)
    {
        //
        // Set up an output stream object to deal with private ofstream copy
        // constructor and lifetime issues.  Let the helper decide the actual
        // name of the file given the prefix.
        //
        AsciiTraceHelper asciiTraceHelper;

        std::string filename;
        if (explicitFilename)
        {
            filename = prefix;
        }
        else
        {
            filename = asciiTraceHelper.GetFilenameFromDevice(prefix, device);
        }

        Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream(filename);

        //
        // The MacRx trace source provides our "r" event.
        //
        asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<SnicNetDevice>(device,
                                                                                     "MacRx",
                                                                                     theStream);

        //
        // The "+", '-', and 'd' events are driven by trace sources actually in the
        // transmit queue.
        //
        // FIXME may be more than one queue for this netdevice
        Ptr<Queue<Packet>> queue = device->GetQueue();
        asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue<Packet>>(queue,
                                                                             "Enqueue",
                                                                             theStream);
        asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue<Packet>>(queue, "Drop", theStream);
        asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue<Packet>>(queue,
                                                                             "Dequeue",
                                                                             theStream);

        // PhyRxDrop trace source for "d" event
        asciiTraceHelper.HookDefaultDropSinkWithoutContext<SnicNetDevice>(device,
                                                                                  "PhyRxDrop",
                                                                                  theStream);

        return;
    }

    //
    // If we are provided an OutputStreamWrapper, we are expected to use it, and
    // to providd a context.  We are free to come up with our own context if we
    // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
    // compatibility and simplicity, we just use Config::Connect and let it deal
    // with the context.
    //
    // Note that we are going to use the default trace sinks provided by the
    // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
    // but the default trace sinks are actually publicly available static
    // functions that are always there waiting for just such a case.
    //
    uint32_t nodeid = nd->GetNode()->GetId();
    uint32_t deviceid = nd->GetIfIndex();
    std::ostringstream oss;

    oss << "/NodeList/" << nd->GetNode()->GetId() << "/DeviceList/" << deviceid
        << "/$ns3::SnicNetDevice/MacRx";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::SnicNetDevice/TxQueue/Enqueue";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::SnicNetDevice/TxQueue/Dequeue";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::SnicNetDevice/TxQueue/Drop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::SnicNetDevice/PhyRxDrop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}
*/

NetDeviceContainer
SnicHelper::Install(Ptr<Node> node, NetDeviceContainer c)
{
  NS_LOG_FUNCTION_NOARGS();

  NetDeviceContainer devs;
  Ptr<SnicNetDevice> dev = m_deviceFactory.Create<SnicNetDevice>();
  NS_LOG_LOGIC("**** Install SNIC device " << dev << " on node " << node->GetId());
  devs.Add(dev);
  node->AddDevice(dev);

  for (NetDeviceContainer::Iterator i = c.Begin(); i != c.End(); ++i)
  {
      NS_LOG_UNCOND("hihi test");
      NS_LOG_LOGIC("**** Add SnicPort " << *i);
      dev->AddSnicPort(*i);
      NS_LOG_LOGIC("**** after Add SnicPort " << *i);
  }
  return devs;
}

NetDeviceContainer
SnicHelper::Install(std::string nodeName, NetDeviceContainer c)
{
    NS_LOG_FUNCTION_NOARGS();
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return Install(node, c);
}

void
SnicHelper::AddPort(Ptr<NetDevice> snic, Ptr<NetDevice> dev)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_LOGIC("**** add port on already created Snic device");
    DynamicCast<SnicNetDevice, NetDevice>(snic)->AddSnicPort(dev, true);
}

void
SnicHelper::ConnectTwoSnic(NetDeviceContainer snic1, NetDeviceContainer snic2)
{
    // Ptr<SnicChannel> ch = CreateObject<SnicChannel>();
    //  NS_ASSERT(snic1.
    Ptr<SnicNetDevice> s1 = DynamicCast<SnicNetDevice, NetDevice>(snic1.Get(0));
    Ptr<SnicNetDevice> s2 = DynamicCast<SnicNetDevice, NetDevice>(snic2.Get(0));
    // s1->AddPeerSnic(s2, ch);
    // s2->AddPeerSnic(s1, ch);
}

void
SnicHelper::CreateAndAggregateObjectFromTypeId(Ptr<NetDevice> device, const std::string typeId)
{
    ObjectFactory factory;
    factory.SetTypeId(typeId);
    Ptr<Object> networkTask = factory.Create<Object>();
    device->AggregateObject(networkTask);
}

void
SnicHelper::CreateSnic(NetDeviceContainer& snics,
                       int num_terminals,
                       NodeContainer& terminals,
                       NodeContainer& csmaSwitches,
                       NetDeviceContainer& terminalDevices,
                       const CsmaHelper& csmaHelper)
{
    NS_LOG_FUNCTION(this << num_terminals);
    NS_LOG_LOGIC("Creating sNIC cluster.");
    // Create new endhost nodes
    NodeContainer newTerminals;
    newTerminals.Create(num_terminals);

    NodeContainer newCsmaSwitch;
    newCsmaSwitch.Create(1);

    NetDeviceContainer newTerminalDevices;
    NetDeviceContainer switchDevices;

    for (int i = 0; i < num_terminals; i++)
    {
        NetDeviceContainer link =
            csmaHelper.Install(NodeContainer(newTerminals.Get(i), newCsmaSwitch));
        terminalDevices.Add(link.Get(0));
        switchDevices.Add(link.Get(1));
    }
    // Create the switch netdevice, which will do the packet switching
    Ptr<Node> switchNode = newCsmaSwitch.Get(0);
    snics.Add(Install(switchNode, switchDevices));

    terminals.Add(newTerminals);
    csmaSwitches.Add(newCsmaSwitch);
    terminalDevices.Add(newTerminalDevices);
}

} // namespace ns3
