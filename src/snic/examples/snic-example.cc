#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/snic-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SnicExample");

int
main(int argc, char* argv[])
{
    bool verbose = true;

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell application to log if true", verbose);

    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    NS_LOG_UNCOND("Hello Simulator");

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));


    /* ... */

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
