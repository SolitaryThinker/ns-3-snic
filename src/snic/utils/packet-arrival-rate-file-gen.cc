#include "packet-arrival-rate-file-gen.h"

#include <cmath>
#include <fstream>

//#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("PacketArrivalRateFileGen");
NS_OBJECT_ENSURE_REGISTERED(PacketArrivalRateFileGen);

TypeId
PacketArrivalRateFileGen::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PacketArrivalRateFileGen")
                            .SetParent<Object>()
                            .SetGroupName("Snic")
                            .AddConstructor<PacketArrivalRateFileGen>();
    //.AddAttribute("Average",
    //"Average in ms of the distribution",
    // TimeValue(MilliSeconds(1000)),
    // MakeTimeAccessor(&PacketArrivalRateFileGen::m_avg),
    // MakeTimeChecker());
    //.AddAttribute("std",
    //"Enable the learning mode of the Learning Bridge",
    // UintegerValue(10),
    // MakeBooleanAccessor(&SnicNetDevice::m_enableLearning),
    // MakeBooleanChecker())
    //.AddAttribute("ExpirationTime",
    //"Time it takes for learned MAC state entry to expire.",
    // TimeValue(Seconds(300)),
    // MakeTimeAccessor(&SnicNetDevice::m_expirationTime),
    // MakeTimeChecker());
    //.AddAttribute("InterframeGap",
    //"The time to wait between packet (frame) transmissions",
    // TimeValue(Seconds(0.0)),
    // MakeTimeAccessor(&SnicNetDevice::m_tInterframeGap),
    // MakeTimeChecker())

    return tid;
}

PacketArrivalRateFileGen::PacketArrivalRateFileGen()
{
}

PacketArrivalRateFileGen::PacketArrivalRateFileGen(std::string fileName)
    : m_fileName(fileName),
      m_currentIdx(0)

{
    NS_LOG_FUNCTION(this);
    std::ifstream file;
    file.open(fileName);

    std::string line;
    if (file.is_open())
    {
        getline(file, line);
        uint64_t n = std::stoi(line);
        getline(file, line);
        double avg = std::stod(line);
        NS_LOG_DEBUG(n);
        NS_LOG_DEBUG(avg);
        while (getline(file, line))
        {
            NS_LOG_DEBUG("line: " << line);
            m_intervals.push_back(std::stoi(line));
        }
    }
    file.close();
}

PacketArrivalRateFileGen::~PacketArrivalRateFileGen()
{
    NS_LOG_FUNCTION_NOARGS();
}

Time
PacketArrivalRateFileGen::NextInterval()
{
    return NanoSeconds(m_intervals[m_currentIdx++]);
}

} // namespace ns3
