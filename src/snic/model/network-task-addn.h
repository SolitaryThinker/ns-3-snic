/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

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

    void ProcessHeader(SnicHeader& header) override;
    void SetIncrement(uint32_t increment);
    uint32_t GetIncrement();

  private:
    uint32_t m_increment; // value added to packet payload
};

} // namespace ns3

#endif // SNIC_NETWORK_TASK_ADDN_H
