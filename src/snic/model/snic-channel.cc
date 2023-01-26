/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-channel.h"

#include "snic-net-device.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicChannel");

NS_OBJECT_ENSURE_REGISTERED(SnicChannel);

TypeId
SnicChannel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SnicChannel")
            .SetParent<Channel>()
            .SetGroupName("Snic")
            .AddConstructor<SnicChannel>()
            .AddAttribute("Delay",
                          "Propagation delay through the channel",
                          TimeValue(Seconds(0)),
                          MakeTimeAccessor(&SnicChannel::m_delay),
                          MakeTimeChecker())
            .AddTraceSource("TxRxSnic",
                            "Trace source indicating transmission of packet "
                            "from the SnicChannel, used by the Animation "
                            "interface.",
                            MakeTraceSourceAccessor(&SnicChannel::m_txrxSnic),
                            "ns3::SnicChannel::TxRxAnimationCallback");
    return tid;
}

SnicChannel::SnicChannel()
    : Channel(),
      m_delay(Seconds(0.)),
      m_nDevices(0)
{
    NS_LOG_FUNCTION_NOARGS();
}

void
SnicChannel::Attach(Ptr<SnicNetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    NS_ASSERT_MSG(m_nDevices < N_DEVICES, "Only two devices permitted");
    NS_ASSERT(device);

    m_link[m_nDevices++].m_src = device;
    //
    // If we have both devices connected to the channel, then finish introducing
    // the two halves and set the links to IDLE.
    //
    if (m_nDevices == N_DEVICES)
    {
        m_link[0].m_dst = m_link[1].m_src;
        m_link[1].m_dst = m_link[0].m_src;
        m_link[0].m_state = IDLE;
        m_link[1].m_state = IDLE;
    }
}

bool
SnicChannel::TransmitStart(Ptr<const Packet> p, Ptr<SnicNetDevice> src, Time txTime)
{
    NS_LOG_FUNCTION(this << p << src);
    NS_LOG_LOGIC("UID is " << p->GetUid() << ")");

    NS_ASSERT(m_link[0].m_state != INITIALIZING);
    NS_ASSERT(m_link[1].m_state != INITIALIZING);

    uint32_t wire = src == m_link[0].m_src ? 0 : 1;

    Simulator::ScheduleWithContext(m_link[wire].m_dst->GetNode()->GetId(),
                                   txTime + m_delay,
                                   &SnicNetDevice::Receive,
                                   m_link[wire].m_dst,
                                   p->Copy());

    // Call the tx anim callback on the net device
    m_txrxSnic(p, src, m_link[wire].m_dst, txTime, txTime + m_delay);
    return true;
}

std::size_t
SnicChannel::GetNDevices() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_nDevices;
}

Ptr<SnicNetDevice>
SnicChannel::GetSnicDevice(std::size_t i) const
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(i < 2);
    return m_link[i].m_src;
}

Ptr<NetDevice>
SnicChannel::GetDevice(std::size_t i) const
{
    NS_LOG_FUNCTION_NOARGS();
    return GetSnicDevice(i);
}

Time
SnicChannel::GetDelay() const
{
    return m_delay;
}

Ptr<SnicNetDevice>
SnicChannel::GetSource(uint32_t i) const
{
    return m_link[i].m_src;
}

Ptr<SnicNetDevice>
SnicChannel::GetDestination(uint32_t i) const
{
    return m_link[i].m_dst;
}

bool
SnicChannel::IsInitialized() const
{
    NS_ASSERT(m_link[0].m_state != INITIALIZING);
    NS_ASSERT(m_link[1].m_state != INITIALIZING);
    return true;
}

} // namespace ns3
