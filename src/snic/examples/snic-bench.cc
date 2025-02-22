#include "ns3/applications-module.h"
#include "ns3/benchmark.h"
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
    Time::SetResolution(Time::NS);
    // LogComponentEnable("SnicExample", LOG_LEVEL_LOGIC);
    // LogComponentEnable("Benchmark", LOG_LEVEL_LOGIC);
    LogComponentEnable("SimpleExperiment", LOG_LEVEL_LOGIC);
    NS_LOG_INFO("Running Benchmark");
    Benchmark benchmark = Benchmark("single-flow");

    std::vector<Ptr<AttributeValue>> values;
    values.push_back(Create<UintegerValue>(400));
    values.push_back(Create<UintegerValue>(500));
    auto v = ExperimentVariable("PacketSize", MakeUintegerChecker<uint32_t>(), values);
    benchmark.AddVariable("PacketSize", v);

    benchmark.Initialize();
    benchmark.Run();
    NS_LOG_INFO("Benchmark done.");
    return 0;
}
