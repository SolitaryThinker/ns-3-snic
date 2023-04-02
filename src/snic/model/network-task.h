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
    static TypeId GetTypeId();

    NetworkTask();
    ~NetworkTask() override;

    virtual void ProcessHeader(SnicHeader& header);

  private:
    uint32_t m_id;
    uint32_t m_ntType;

    double m_fpgaFabric;
    DataRate m_ingressBps;
    DataRate m_egressBps;
    double m_memoryRequirement;

    uint32_t m_pipelineSize;
    Time m_delay;

    // memory usage model

    // used during deployment
    bool m_ready;
    bool m_performingReconfig;
};

} // namespace ns3

#endif // SNIC_NETWORK_TASK_H
