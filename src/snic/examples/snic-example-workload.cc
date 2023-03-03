#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/ring-topology.h"
#include "ns3/snic-helper.h"
#include "ns3/snic-net-device.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SnicExample");

/* two snics using snic apps and sockets */
int
main(int argc, char* argv[])
{
    bool verbose = true;

    LogComponentEnable("SnicExample", LOG_LEVEL_LOGIC);
    // LogComponentEnable("PacketBuffer", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicHelper", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("SnicChannel", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("Node", LOG_LEVEL_LOGIC);
    // LogComponentEnable("ArpL3Protocol", LOG_LEVEL_LOGIC);
    // LogComponentEnable("ArpCache", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("SnicStackHelper", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicScheduler", LOG_LEVEL_LOGIC);
    // LogComponentEnable("Ipv4AddressHelper", LOG_LEVEL_LOGIC);
    //    LogComponentEnable("Ipv4", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("SnicL4Protocol", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicNetDevice", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicWorkloadClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("SnicWorkloadServerApplication", LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell application to log if true", verbose);

    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    RingTopologyHelper ringHelper = RingTopologyHelper(2, 1, 0);

    NodeContainer terminals = ringHelper.GetTerminals();

    NS_LOG_INFO("Create Applications.");

    Ipv4InterfaceContainer interfaces = ringHelper.GetInterfaces();
    // uint16_t port = 9; // Discard port (RFC 863)

    SnicWorkloadServerHelper echoServer(9);

    ApplicationContainer serverApps2 = echoServer.Install(terminals.Get(0));
    serverApps2.Start(Seconds(1.0));
    serverApps2.Stop(Seconds(10.0));

    SnicWorkloadClientHelper echoClient2(interfaces.GetAddress(0), 9);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient2.SetAttribute("Interval", TimeValue(MilliSeconds(40.0)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    // ApplicationContainer clientApps = echoClient2.Install(terminals.Get(2));
    ApplicationContainer clientApps2 = echoClient2.Install(terminals.Get(1));
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
    CsmaHelper csmaHelper = ringHelper.GetCsmaHelper();
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
