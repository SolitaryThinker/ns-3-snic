#include "statistic.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Statistic");
NS_OBJECT_ENSURE_REGISTERED(Statistic);

TypeId
Statistic::GetTypeId(){
    static TypeId tid = TypeId("ns3::Statistic")
                            .SetParent<Object>()
                            .SetGroupName("Snic")
                            .AddConstructor<Statistic>();
    return tid;
}

Statistic::Statistic()
    : m_tput(0),
      m_total_tput(0),
      m_count(0),
      m_first(true)
{
}

Statistic::~Statistic()
{
}

double
Statistic::GetTput() const
{
    NS_LOG_FUNCTION(this);
    return m_tput;
}

void
Statistic::AddPacket(uint32_t size)
{
    NS_LOG_FUNCTION(this << size);
    if (!m_first)
    {
        uint64_t lat = Simulator::Now().GetNanoSeconds() - m_prev.GetNanoSeconds();
        m_prev = Simulator::Now();
        m_tput = (double)size / lat;
        m_total_tput += m_tput;
        m_count++;
        double avg_lat = m_total_tput / m_count;
        NS_LOG_INFO("now=" << m_prev);
        NS_LOG_INFO("lat=" << lat << " m_tput=" << m_tput);
        NS_LOG_INFO("avg lat=" << avg_lat);
    }
    else
    {
        m_prev = Simulator::Now();
        m_first = false;
    }
}

} // namespace ns3
