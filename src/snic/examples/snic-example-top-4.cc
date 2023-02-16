#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/snic-helper.h"
#include "ns3/snic-net-device.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SnicExample");

/* Create a new snic connected to num_terminals endhosts */
void
CreateSnic(NetDeviceContainer& snics,
           int num_terminals,
           NodeContainer& terminals,
           NodeContainer& csmaSwitches,
           NetDeviceContainer& terminalDevices,
           const CsmaHelper& csmaHelper,
           SnicHelper& snicHelper)
{
    NS_LOG_FUNCTION(num_terminals);
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
    snics.Add(snicHelper.Install(switchNode, switchDevices));

    terminals.Add(newTerminals);
    csmaSwitches.Add(newCsmaSwitch);
    terminalDevices.Add(newTerminalDevices);
}

/* two snics using snic apps and sockets */
int
main(int argc, char* argv[])
{
    bool verbose = true;

    LogComponentEnable("SnicExample", LOG_LEVEL_LOGIC);
    LogComponentEnable("PacketBuffer", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicHelper", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("SnicChannel", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("Node", LOG_LEVEL_LOGIC);
    LogComponentEnable("ArpL3Protocol", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicStackHelper", LOG_LEVEL_LOGIC);
    // LogComponentEnable("Ipv4AddressHelper", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("Ipv4", LOG_LEVEL_LOGIC);
    LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("SnicL4Protocol", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicNetDevice", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("SnicEchoServerApplication", LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell application to log if true", verbose);

    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    NS_LOG_UNCOND("Hello Simulator");

    // helper classes
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    SnicHelper snicHelper;
    // swtch.SetChannelAttribute("DataRate", DataRateValue(5000000));
    // swtch.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    // Containers
    // List of end host nodes
    NodeContainer terminals;
    // List of csma devices at each end host
    NetDeviceContainer terminalDevices;
    // List of sNICs
    NetDeviceContainer snics;
    // List of Nodes for switch
    NodeContainer csmaSwitches;

    // first sNIC cluster
    NS_LOG_INFO("Creating first sNIC cluster.");

    CreateSnic(snics, 4, terminals, csmaSwitches, terminalDevices, csmaHelper, snicHelper);
    NS_LOG_INFO("Creating second sNIC cluster.");
    CreateSnic(snics, 4, terminals, csmaSwitches, terminalDevices, csmaHelper, snicHelper);
    NS_LOG_INFO("Creating 3rd sNIC cluster.");
    CreateSnic(snics, 4, terminals, csmaSwitches, terminalDevices, csmaHelper, snicHelper);
    NS_LOG_INFO("Creating 4th sNIC cluster.");
    CreateSnic(snics, 4, terminals, csmaSwitches, terminalDevices, csmaHelper, snicHelper);

    // connect two snics together
    NetDeviceContainer snicLink = csmaHelper.Install(csmaSwitches);
    // swtch.ConnectTwoSnic(snic1, snic2);
    // swtch.Install(switchNode, snicLink.Get(0));
    // swtch2.Install(switchNode2, snicLink.Get(1));
    snicHelper.AddPort(snics.Get(0), snicLink.Get(0));
    snicHelper.AddPort(snics.Get(1), snicLink.Get(1));
    snicHelper.AddPort(snics.Get(2), snicLink.Get(2));
    snicHelper.AddPort(snics.Get(3), snicLink.Get(3));

    // Add internet stack to the terminals
    SnicStackHelper internet;
    internet.Install(terminals);
    internet.Install(csmaSwitches);

    // We've got the "hardware" in place.  Now we need to add IP addresses.
    //
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(terminalDevices);
    // NS_LOG_INFO("snics helper: " << snics);
    Ipv4InterfaceContainer snic_interfaces = ipv4.Assign(snics);

    // XXX set 4th snic to be scheduler manually
    for (NetDeviceContainer::Iterator i = snics.Begin(); i != snics.End(); ++i)
    {
        NS_LOG_LOGIC("snic_ptr " << *i);
        NS_LOG_LOGIC("addresses ");
        Ptr<SnicNetDevice> snic = DynamicCast<SnicNetDevice, NetDevice>(*i);
        snic->SetSchedulerAddress(snic_interfaces.GetAddress(0));
        snic->SetIpAddress(snic_interfaces.GetAddress(i - snics.Begin()));
    }
    DynamicCast<SnicNetDevice, NetDevice>(snics.Get(0))->SetIsScheduler(true);

    NS_LOG_INFO("Create Applications.");
    // uint16_t port = 9; // Discard port (RFC 863)

    SnicEchoServerHelper echoServer(9);

    ApplicationContainer serverApps2 = echoServer.Install(terminals.Get(1));
    serverApps2.Start(Seconds(1.0));
    serverApps2.Stop(Seconds(10.0));

    SnicEchoClientHelper echoClient2(interfaces.GetAddress(1), 9);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    // ApplicationContainer clientApps = echoClient2.Install(terminals.Get(2));
    ApplicationContainer clientApps2 = echoClient2.Install(terminals.Get(14));
    clientApps2.Start(Seconds(2.0));
    clientApps2.Stop(Seconds(10.0));
    //   clientApps.Start(Seconds(2.0));
    //   clientApps.Stop(Seconds(10.0));

    NS_LOG_INFO("Configure Tracing.");

    //
    // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
    // Trace output will be sent to the file "csma-bridge.tr"
    //
    AsciiTraceHelper ascii;
    csmaHelper.EnableAsciiAll(ascii.CreateFileStream("csma-bridge.tr"));

    //
    // Also configure some tcpdump traces; each interface will be traced.
    // The output files will be named:
    //     csma-bridge-<nodeId>-<interfaceId>.pcap
    // and can be read by the "tcpdump -r" command (use "-tt" option to
    // display timestamps correctly)
    //
    csmaHelper.EnablePcapAll("csma-bridge", false);

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;
}
