/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "network-task.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NetworkTask");
NS_OBJECT_ENSURE_REGISTERED(NetworkTask);

TypeId
NetworkTask::GetTypeId(){
    static TypeId tid = TypeId("ns3::NetworkTask")
                            .SetParent<Object>()
                            .SetGroupName("Snic")
                            .AddConstructor<NetworkTask>();
    //.AddAttribute("Mtu",
    //"The MAC-level Maximum Transmission Unit",
    // UintegerValue(DEFAULT_MTU),
    // MakeUintegerAccessor(&SnicNetDevice::SetMtu, &SnicNetDevice::GetMtu),
    // MakeUintegerChecker<uint16_t>());
    return tid;
}

NetworkTask::NetworkTask()
{
}

NetworkTask::~NetworkTask()
{
}

void
NetworkTask::ProcessHeader(SnicHeader& header)
{
    NS_FATAL_ERROR("shell NTs can't process packets");
}

} // namespace ns3

