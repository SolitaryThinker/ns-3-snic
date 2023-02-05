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
    // LogComponentEnable("SnicHelper", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicNetDevice", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("SnicEchoServerApplication", LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell application to log if true", verbose);

    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    NS_LOG_UNCOND("Hello Simulator");

    NS_LOG_INFO("Create nodes.");
    NodeContainer terminals;
    terminals.Create(4);

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
    NS_LOG_UNCOND("2  222 csma Simulator");

    // Create the switch netdevice, which will do the packet switching
    Ptr<Node> switchNode = csmaSwitch.Get(0);
    SnicHelper swtch;
    NS_LOG_UNCOND("csma Simulator");
    swtch.Install(switchNode, switchDevices);

    // Add internet stack to the terminals
    SnicStackHelper internet;
    internet.Install(terminals);

    // We've got the "hardware" in place.  Now we need to add IP addresses.
    //
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(terminalDevices);

    NS_LOG_INFO("Create Applications.");
    // uint16_t port = 9; // Discard port (RFC 863)

    SnicEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(terminals.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    SnicEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(terminals.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

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
