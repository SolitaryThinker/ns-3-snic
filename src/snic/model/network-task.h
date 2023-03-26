/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_NETWORK_TASK_H
#define SNIC_NETWORK_TASK_H

#include "ns3/data-rate.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/snic-header.h"

namespace ns3
{

class NetworkTask : public Object
{
  public:
    NetworkTask();
    ~NetworkTask();

    virtual void ProcessHeader(SnicHeader& header) = 0;

  private:
    DataRate m_ingressBps;
    DataRate m_egressBps;
    double m_memoryRequirement;
};

} // namespace ns3

#endif // SNIC_NETWORK_TASK_H
