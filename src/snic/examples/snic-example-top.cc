#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/snic-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SnicExample");

int
main(int argc, char* argv[])
{
    bool verbose = true;

    // LogComponentEnable("SnicExample", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("SnicHelper", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicChannel", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicNetDevice", LOG_LEVEL_LOGIC);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell application to log if true", verbose);

    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    NS_LOG_UNCOND("Hello Simulator");

    // first sNIC cluster
    NS_LOG_INFO("Creating first sNIC cluster.");
    NS_LOG_INFO("Create nodes.");
    NodeContainer terminals;
    terminals.Create(8);

    NodeContainer csmaSwitch;
    csmaSwitch.Create(1);

    NS_LOG_INFO("Build Topology");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    // Create the csma links, from each terminal to the switch

    NetDeviceContainer terminalDevices;
    NetDeviceContainer switchDevices;

    for (int i = 0; i < 4; i++)
    {
        NetDeviceContainer link = csma.Install(NodeContainer(terminals.Get(i), csmaSwitch));
        terminalDevices.Add(link.Get(0));
        switchDevices.Add(link.Get(1));
    }
    // Create the switch netdevice, which will do the packet switching
    Ptr<Node> switchNode = csmaSwitch.Get(0);
    SnicHelper swtch;
    // swtch.SetChannelAttribute("DataRate", DataRateValue(5000000));
    // swtch.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
    NS_LOG_UNCOND("csma Simulator");
    NetDeviceContainer snics = swtch.Install(switchNode, switchDevices);

    //
    // second sNIC cluster
    //
    NS_LOG_INFO("Creating second sNIC cluster.");
    NS_LOG_INFO("Create nodes.");

    NodeContainer csmaSwitch2;
    csmaSwitch2.Create(1);

    NS_LOG_INFO("Build Topology");

    // Create the csma links, from each terminal to the switch

    NetDeviceContainer terminalDevices2;
    NetDeviceContainer switchDevices2;

    for (int i = 4; i < 8; i++)
    {
        NetDeviceContainer link = csma.Install(NodeContainer(terminals.Get(i), csmaSwitch2));
        terminalDevices2.Add(link.Get(0));
        switchDevices2.Add(link.Get(1));
    }

    Ptr<Node> switchNode2 = csmaSwitch2.Get(0);
    NS_LOG_UNCOND("csma Simulator");
    snics.Add(swtch.Install(switchNode2, switchDevices2));

    NetDeviceContainer snicLink = csma.Install(NodeContainer(csmaSwitch, csmaSwitch2));
    // swtch.ConnectTwoSnic(snic1, snic2);
    // swtch.Install(switchNode, snicLink.Get(0));
    // swtch2.Install(switchNode2, snicLink.Get(1));
    swtch.AddPort(snics.Get(0), snicLink.Get(0));
    swtch.AddPort(snics.Get(1), snicLink.Get(1));

    // Add internet stack to the terminals
    InternetStackHelper internet;
    internet.Install(terminals);

    // We've got the "hardware" in place.  Now we need to add IP addresses.
    //
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(terminalDevices);
    Ipv4InterfaceContainer interfaces2 = ipv4.Assign(terminalDevices2);

    NS_LOG_INFO("Create Applications.");
    // uint16_t port = 9; // Discard port (RFC 863)

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps2 = echoServer.Install(terminals.Get(1));
    serverApps2.Start(Seconds(1.0));
    serverApps2.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient2(interfaces.GetAddress(1), 9);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    // ApplicationContainer clientApps = echoClient2.Install(terminals.Get(2));
    ApplicationContainer clientApps2 = echoClient2.Install(terminals.Get(6));
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
    csma.EnableAsciiAll(ascii.CreateFileStream("csma-bridge.tr"));

    //
    // Also configure some tcpdump traces; each interface will be traced.
    // The output files will be named:
    //     csma-bridge-<nodeId>-<interfaceId>.pcap
    // and can be read by the "tcpdump -r" command (use "-tt" option to
    // display timestamps correctly)
    //
    csma.EnablePcapAll("csma-bridge", false);

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;
}
