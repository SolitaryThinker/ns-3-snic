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
uint64_t totalSchedPackets = 0;
Time prev;

void
IntTrace(std::string context, uint64_t old, uint64_t newv)
{
    NS_LOG_DEBUG("traced req " << context << " " << old << " to " << newv);
    Time now = Simulator::Now();
    NS_LOG_DEBUG("sim time: " << now - prev);
    prev = now;


    totalSchedPackets = newv;
}

Statistic schedStat;

void
SchedTraceC(std::string context, Ptr<const SnicNetDevice> dev, Ptr<const Packet> pkt)
{
    NS_LOG_DEBUG(context << " isSched=" << dev->IsScheduler()
                         << " numSchedReq=" << dev->GetNumSchedReqs());
}

void
SchedTrace(Ptr<const SnicNetDevice> dev, Ptr<const Packet> pkt)
{
    NS_LOG_DEBUG("isSched=" << dev->IsScheduler() << " numSchedReq=" << dev->GetNumSchedReqs()
                            << " " << pkt->GetUid());
    schedStat.AddPacket(pkt->GetSize());
}

/* two snics using snic apps and sockets */
int
main(int argc, char* argv[])
{
    bool verbose = true;

    LogComponentEnable("SnicExample", LOG_LEVEL_LOGIC);
    // LogComponentEnable("CsmaNetDevice", LOG_LEVEL_LOGIC);
    // LogComponentEnable("CsmaChannel", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("DataRate", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("FlowId", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicSchedulerHeader", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicHeader", LOG_LEVEL_LOGIC);
    LogComponentEnable("Statistic", LOG_LEVEL_LOGIC);
    // LogComponentEnable("PacketBuffer", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicHelper", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("SnicChannel", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("Node", LOG_LEVEL_LOGIC);
    // LogComponentEnable("ArpL3Protocol", LOG_LEVEL_LOGIC);
    // LogComponentEnable("ArpCache", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("SnicStackHelper", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicScheduler", LOG_LEVEL_LOGIC);
    // LogComponentEnable("Ipv4AddressHelper", LOG_LEVEL_LOGIC);
    //    LogComponentEnable("Ipv4", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicL4Protocol", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicNetDevice", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicWorkloadClientApplication", LOG_LEVEL_LOGIC);
    LogComponentEnable("SnicWorkloadServerApplication", LOG_LEVEL_LOGIC);

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell application to log if true", verbose);

    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    RingTopologyHelper ringHelper = RingTopologyHelper(4, 1, 0);

    NodeContainer terminals = ringHelper.GetTerminals();

    NS_LOG_INFO("Create Applications.");

    Ipv4InterfaceContainer interfaces = ringHelper.GetInterfaces();
    // uint16_t port = 9; // Discard port (RFC 863)

    SnicWorkloadServerHelper workloadServer(9);

    ApplicationContainer serverApps2 = workloadServer.Install(terminals.Get(0));
    serverApps2.Start(Seconds(1.0));
    serverApps2.Stop(Seconds(20.0));

    SnicWorkloadClientHelper workloadClient(interfaces.GetAddress(0), 9);
    workloadClient.SetAttribute("MaxPackets", UintegerValue(4490));
    workloadClient.SetAttribute("Interval", TimeValue(NanoSeconds(4.0)));
    workloadClient.SetAttribute("PacketSize", UintegerValue(450));
    // workloadClient.SetAttribute("PacketSize", UintegerValue(9));
    workloadClient.SetAttribute("UseFlow", BooleanValue(true));
    workloadClient.SetAttribute("FlowSize", UintegerValue(900));
    workloadClient.SetAttribute("FlowPktCount", UintegerValue(50));

    // workloadClient.SetAttribute("Gen", FileGen("trace.txt"));

    // ApplicationContainer clientApps = workloadClient.Install(terminals.Get(2));
    ApplicationContainer clientApps2 = workloadClient.Install(terminals.Get(1));
    clientApps2.Start(Seconds(2.0));
    auto client = clientApps2.Get(0);
    PacketArrivalRateGen gen = PacketArrivalRateFileGen("trace.txt");
    client.SetPktGen(pktGen);
    // clientApps2.Stop(Seconds(3.0));

    // Ptr<SnicWorkloadServer> server =
    // DynamicCast<SnicWorkloadServer, Application>(serverApps2.Get(0));
    // server->Reset();
    // clientApps2.Start(Seconds(3.1));
    clientApps2.Stop(Seconds(20.0));
    //   clientApps.Start(Seconds(2.0));
    //   clientApps.Stop(Seconds(10.0));

    NS_LOG_INFO("Configure Tracing.");
    NetDeviceContainer snics = ringHelper.GetSnics();
    // snics.Get(0)->TraceConnectWithoutContext("NumL4Packets", MakeCallback(&IntTrace));
    snics.Get(0)->TraceConnectWithoutContext("SchedTrace", MakeCallback(&SchedTrace));
    // snics.Get(0)->TraceConnect("SchedTrace", MakeCallback(&SchedTraceC));

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

    NS_LOG_INFO("totalSchedPackets: " << totalSchedPackets);

    return 0;
}
