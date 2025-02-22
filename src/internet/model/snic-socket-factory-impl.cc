/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-socket-factory-impl.h"

#include "snic-l4-protocol.h"

#include "ns3/assert.h"
#include "ns3/socket.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicSocketFactoryImpl");

SnicSocketFactoryImpl::SnicSocketFactoryImpl()
    : m_snic(nullptr)
{
}

SnicSocketFactoryImpl::~SnicSocketFactoryImpl()
{
    NS_ASSERT(!m_snic);
}

void
SnicSocketFactoryImpl::SetSnic(Ptr<SnicL4Protocol> snic)
{
    m_snic = snic;
}

Ptr<Socket>
SnicSocketFactoryImpl::CreateSocket()
{
    return m_snic->CreateSocket();
}

void
SnicSocketFactoryImpl::DoDispose()
{
    m_snic = nullptr;
    SnicSocketFactory::DoDispose();
}

} // namespace ns3
