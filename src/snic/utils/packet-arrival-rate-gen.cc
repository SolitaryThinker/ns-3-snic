#include "packet-arrival-rate-gen.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("PacketArrivalRateGen");
NS_OBJECT_ENSURE_REGISTERED(PacketArrivalRateGen);

TypeId
PacketArrivalRateGen::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PacketArrivalRateGen")
                            .SetParent<Object>()
                            .SetGroupName("Snic")
                            .AddConstructor<PacketArrivalRateGen>();
    //.AddAttribute("Average",
    //"The MAC-level Maximum Transmission Unit",
    // UintegerValue(5),
    // MakeUintegerAccessor(&SnicNetDevice::SetMtu, &SnicNetDevice::GetMtu),
    // MakeUintegerChecker<uint16_t>())
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

PacketArrivalRateGen::PacketArrivalRateGen()
    : m_isPeaking(false),
      m_avg(5),
      m_std(5),
      m_distribution(5, 5)
{
    NS_LOG_FUNCTION(this << 5 << 5);
}

PacketArrivalRateGen::PacketArrivalRateGen(uint64_t avg, uint64_t std)
    : m_isPeaking(false),
      m_avg(avg),
      m_std(std),
      m_distribution(avg, std)
{
    NS_LOG_FUNCTION(this << m_avg << m_std);
}

PacketArrivalRateGen::~PacketArrivalRateGen()
{
    NS_LOG_FUNCTION_NOARGS();
}

Time
PacketArrivalRateGen::NextInterval()
{
    return NanoSeconds(m_distribution(m_generator));
}

} // namespace ns3
