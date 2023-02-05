/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-socket-factory.h"

#include "ns3/log.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicSocketFactory");

NS_OBJECT_ENSURE_REGISTERED(SnicSocketFactory);

TypeId
SnicSocketFactory::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SnicSocketFactory").SetParent<SocketFactory>().SetGroupName("Internet");
    return tid;
}

} // namespace ns3
