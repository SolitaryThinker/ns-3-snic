#ifndef PACKET_ARRIVAL_RATE_FILE_GEN_H
#define PACKET_ARRIVAL_RATE_FILE_GEN_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet-arrival-rate-gen.h"

#include <random>
#include <string>

namespace ns3
{

class PacketArrivalRateFileGen : public PacketArrivalRateGen
{
  public:
    static TypeId GetTypeId();

    PacketArrivalRateFileGen();
    PacketArrivalRateFileGen(std::string fileName);
    ~PacketArrivalRateFileGen();

    Time NextInterval() override;

    // void SetAverage(uint64_t avg);
    // void SetStd(uint64_t std);

  private:
    // bool m_isPeaking;
    std::string m_fileName;
    std::vector<uint32_t> m_intervals;
    uint64_t m_currentIdx;
    // uint64_t m_avg;
    // uint64_t m_std;
    // std::default_random_engine m_generator;
    // std::normal_distribution<double> m_distribution;
};

} // namespace ns3

#endif // PACKET_ARRIVAL_RATE_GEN_H
