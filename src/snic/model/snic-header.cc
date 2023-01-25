/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "llc-snap-header.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicHeader");

NS_OBJECT_ENSURE_REGISTERED(SnicHeader);

SnicHeader::SnicHeader()
{
    NS_LOG_FUNCTION(this);
}
} // namespace ns3
