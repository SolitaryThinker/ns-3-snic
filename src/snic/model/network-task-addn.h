#ifndef SNIC_NETWORK_TASK_ADDN_H
#define SNIC_NETWORK_TASK_ADDN_H

#include "network-task.h"

namespace ns3
{

class NetworkTaskAddN : public NetworkTask
{
  public:
    NetworkTaskAddN();
    ~NetworkTaskAddN();

    void ProcessPacket(Ptr<Packet> packet) override;

  private:
    uint32_t m_increment; // value added to packet payload
};

} // namespace ns3

#endif // SNIC_NETWORK_TASK_ADDN_H
