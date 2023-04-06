#include "simple-experiment.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/ring-topology.h"
#include "ns3/snic-helper.h"
#include "ns3/snic-net-device.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SimpleExperiment");
NS_OBJECT_ENSURE_REGISTERED(SimpleExperiment);

SimpleExperiment::SimpleExperiment(uint32_t id, std::string prefix)
    : Experiment(id, prefix)
{
}

SimpleExperiment::~SimpleExperiment()
{
}

void
SimpleExperiment::Initialize(std::map<std::string, std::vector<Ptr<AttributeValue>>> variables,
                             std::map<std::string, uint32_t> indexes)
{
    NS_LOG_FUNCTION(this);
    // m_packetSize =

    // LogComponentEnable("CsmaNetDevice", LOG_LEVEL_LOGIC);
    // LogComponentEnable("CsmaChannel", LOG_LEVEL_LOGIC);
    //  LogComponentEnable("DataRate", LOG_LEVEL_LOGIC);
    //   LogComponentEnable("FlowId", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicSchedulerHeader", LOG_LEVEL_LOGIC);
    // LogComponentEnable("PacketArrivalRateFileGen", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicHeader", LOG_LEVEL_LOGIC);
    // LogComponentEnable("Statistic", LOG_LEVEL_LOGIC);
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
    // LogComponentEnable("SnicL4Protocol", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicNetDevice", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicWorkloadClientApplication", LOG_LEVEL_LOGIC);
    // LogComponentEnable("SnicWorkloadServerApplication", LOG_LEVEL_LOGIC);

    RingTopologyHelper ringHelper = RingTopologyHelper(4, 1, 0);

    NodeContainer terminals = ringHelper.GetTerminals();

    NS_LOG_INFO("Create Applications.");

    Ipv4InterfaceContainer interfaces = ringHelper.GetInterfaces();
    // uint16_t port = 9; // Discard port (RFC 863)

    SnicWorkloadServerHelper workloadServer(9);

    ApplicationContainer serverApps = workloadServer.Install(terminals.Get(0));

    Ptr<SnicWorkloadServer> server =
        DynamicCast<SnicWorkloadServer, Application>(serverApps.Get(0));
    // Ptr<PacketArrivalRateGen> gen = Create<PacketArrivalRateFileGen>("trace.txt");
    NS_LOG_DEBUG(m_outputFileName);
    server->SetOutputFile(m_outputFileName);

    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(20.0));

    SnicWorkloadClientHelper workloadClient(interfaces.GetAddress(0), 9);
    workloadClient.SetAttribute("MaxPackets", UintegerValue(990));
    workloadClient.SetAttribute("Interval", TimeValue(NanoSeconds(4.0)));
    // workloadClient.SetAttribute("PacketSize", UintegerValue(450));
    //  workloadClient.SetAttribute("PacketSize", UintegerValue(9));
    workloadClient.SetAttribute("UseFlow", BooleanValue(true));
    workloadClient.SetAttribute("FlowSize", UintegerValue(900));
    workloadClient.SetAttribute("FlowPktCount", UintegerValue(990));

    for (const auto& kv : variables)
    {
        workloadClient.SetAttribute(kv.first, *kv.second[indexes[kv.first]]);
    }

    // workloadClient.SetAttribute("Gen", FileGen("trace.txt"));

    // ApplicationContainer clientApps = workloadClient.Install(terminals.Get(2));
    ApplicationContainer clientApps2 = workloadClient.Install(terminals.Get(1));
    Ptr<SnicWorkloadClient> client =
        DynamicCast<SnicWorkloadClient, Application>(clientApps2.Get(0));
    Ptr<PacketArrivalRateGen> gen = Create<PacketArrivalRateFileGen>("trace.txt");
    NS_LOG_DEBUG("gen");
    client->SetPktGen(gen);

    clientApps2.Start(Seconds(2.0));
    //  clientApps2.Stop(Seconds(3.0));

    // Ptr<SnicWorkloadServer> server =
    // DynamicCast<SnicWorkloadServer, Application>(serverApps.Get(0));
    // server->Reset();
    // clientApps2.Start(Seconds(3.1));
    clientApps2.Stop(Seconds(20.0));
    //   clientApps.Start(Seconds(2.0));
    //   clientApps.Stop(Seconds(10.0));

    NS_LOG_INFO("Configure Tracing.");
    NetDeviceContainer snics = ringHelper.GetSnics();
    // snics.Get(0)->TraceConnectWithoutContext("NumL4Packets", MakeCallback(&IntTrace));
    // snics.Get(0)->TraceConnectWithoutContext("SchedTrace", MakeCallback(&SchedTrace));
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
    NS_LOG_FUNCTION(this);
    // NS_ASSERT(m_initialized);
    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}

void
SimpleExperiment::Run()
{
}
} // namespace ns3
