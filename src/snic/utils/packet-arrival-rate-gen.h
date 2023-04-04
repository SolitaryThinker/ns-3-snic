#ifndef PACKET_ARRIVAL_RATE_GEN_H
#define PACKET_ARRIVAL_RATE_GEN_H

#include "ns3/nstime.h"
#include "ns3/object.h"

#include <random>

namespace ns3
{

class PacketArrivalRateGen : public Object
{
  public:
    static TypeId GetTypeId();

    PacketArrivalRateGen();
    PacketArrivalRateGen(uint64_t avg, uint64_t std);
    ~PacketArrivalRateGen();
    virtual Time NextInterval();

    void SetAverage(uint64_t avg);
    void SetStd(uint64_t std);

  private:
    bool m_isPeaking;
    uint64_t m_avg;
    uint64_t m_std;
    std::default_random_engine m_generator;
    std::normal_distribution<double> m_distribution;
};

} // namespace ns3

#endif // PACKET_ARRIVAL_RATE_GEN_H
