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
            .AddAttribute(
                "DataRate",
                "The transmission data rate to be provided to devices connected to the channel",
                DataRateValue(DataRate(0xffffffff)),
                MakeDataRateAccessor(&SnicChannel::m_bps),
                MakeDataRateChecker())
            .AddAttribute("Delay",
                          "Transmission delay through the channel",
                          TimeValue(Seconds(0)),
                          MakeTimeAccessor(&SnicChannel::m_delay),
                          MakeTimeChecker());
    return tid;
}

SnicChannel::SnicChannel()
    : Channel()
{
    NS_LOG_FUNCTION_NOARGS();
    m_state = IDLE;
    m_deviceList.clear();
}

SnicChannel::~SnicChannel()
{
    NS_LOG_FUNCTION(this);
    m_deviceList.clear();
}

int32_t
SnicChannel::Attach(Ptr<SnicNetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    NS_ASSERT(device);

    SnicDeviceRec rec(device);

    m_deviceList.push_back(rec);
    return (m_deviceList.size() - 1);
}

bool
SnicChannel::Reattach(Ptr<SnicNetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    NS_ASSERT(device);

    std::vector<SnicDeviceRec>::iterator it;
    for (it = m_deviceList.begin(); it < m_deviceList.end(); it++)
    {
        if (it->devicePtr == device)
        {
            if (!it->active)
            {
                it->active = true;
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool
SnicChannel::Reattach(uint32_t deviceId)
{
    NS_LOG_FUNCTION(this << deviceId);

    if (deviceId < m_deviceList.size())
    {
        return false;
    }

    if (m_deviceList[deviceId].active)
    {
        return false;
    }
    else
    {
        m_deviceList[deviceId].active = true;
        return true;
    }
}

bool
SnicChannel::Detach(uint32_t deviceId)
{
    NS_LOG_FUNCTION(this << deviceId);

    if (deviceId < m_deviceList.size())
    {
        if (!m_deviceList[deviceId].active)
        {
            NS_LOG_WARN("SnicChannel::Detach(): Device is already detached (" << deviceId << ")");
            return false;
        }

        m_deviceList[deviceId].active = false;

        if ((m_state == TRANSMITTING) && (m_currentSrc == deviceId))
        {
            NS_LOG_WARN("SnicChannel::Detach(): Device is currently"
                        << "transmitting (" << deviceId << ")");
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool
SnicChannel::Detach(Ptr<SnicNetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    NS_ASSERT(device);

    std::vector<SnicDeviceRec>::iterator it;
    for (it = m_deviceList.begin(); it < m_deviceList.end(); it++)
    {
        if ((it->devicePtr == device) && (it->active))
        {
            it->active = false;
            return true;
        }
    }
    return false;
}

bool
SnicChannel::TransmitStart(Ptr<const Packet> p, uint32_t srcId)
{
    NS_LOG_FUNCTION(this << p << srcId);
    NS_LOG_INFO("UID is " << p->GetUid() << ")");

    if (m_state != IDLE)
    {
        NS_LOG_WARN("SnicChannel::TransmitStart(): State is not IDLE");
        return false;
    }

    if (!IsActive(srcId))
    {
        NS_LOG_ERROR(
            "SnicChannel::TransmitStart(): Seclected source is not currently attached to network");
        return false;
    }

    NS_LOG_LOGIC("switch to TRANSMITTING");
    m_currentPkt = p->Copy();
    m_currentSrc = srcId;
    m_state = TRANSMITTING;
    return true;
}

bool
SnicChannel::IsActive(uint32_t deviceId)
{
    return (m_deviceList[deviceId].active);
}

bool
SnicChannel::TransmitEnd()
{
    NS_LOG_FUNCTION(this << m_currentPkt << m_currentSrc);
    NS_LOG_INFO("UID is " << m_currentPkt->GetUid() << ")");

    NS_ASSERT(m_state == TRANSMITTING);
    m_state = PROPAGATING;

    bool retVal = true;

    if (!IsActive(m_currentSrc))
    {
        NS_LOG_ERROR("SnicChannel::TransmitEnd(): Seclected source was detached before the end of "
                     "the transmission");
        retVal = false;
    }

    NS_LOG_LOGIC("Schedule event in " << m_delay.As(Time::S));

    NS_LOG_LOGIC("Receive");

    std::vector<SnicDeviceRec>::iterator it;
    for (it = m_deviceList.begin(); it < m_deviceList.end(); it++)
    {
        if (it->IsActive() && it->devicePtr != m_deviceList[m_currentSrc].devicePtr)
        {
            // schedule reception events
            Simulator::ScheduleWithContext(it->devicePtr->GetNode()->GetId(),
                                           m_delay,
                                           &SnicNetDevice::Receive,
                                           it->devicePtr,
                                           m_currentPkt->Copy(),
                                           m_deviceList[m_currentSrc].devicePtr);
        }
    }

    // also schedule for the tx side to go back to IDLE
    Simulator::Schedule(m_delay, &SnicChannel::PropagationCompleteEvent, this);
    return retVal;
}

void
SnicChannel::PropagationCompleteEvent()
{
    NS_LOG_FUNCTION(this << m_currentPkt);
    NS_LOG_INFO("UID is " << m_currentPkt->GetUid() << ")");

    NS_ASSERT(m_state == PROPAGATING);
    m_state = IDLE;
}

uint32_t
SnicChannel::GetNumActDevices()
{
    int numActDevices = 0;
    std::vector<SnicDeviceRec>::iterator it;
    for (it = m_deviceList.begin(); it < m_deviceList.end(); it++)
    {
        if (it->active)
        {
            numActDevices++;
        }
    }
    return numActDevices;
}

std::size_t
SnicChannel::GetNDevices() const
{
    return m_deviceList.size();
}

Ptr<SnicNetDevice>
SnicChannel::GetSnicDevice(std::size_t i) const
{
    return m_deviceList[i].devicePtr;
}

int32_t
SnicChannel::GetDeviceNum(Ptr<SnicNetDevice> device)
{
    std::vector<SnicDeviceRec>::iterator it;
    int i = 0;
    for (it = m_deviceList.begin(); it < m_deviceList.end(); it++)
    {
        if (it->devicePtr == device)
        {
            if (it->active)
            {
                return i;
            }
            else
            {
                return -2;
            }
        }
        i++;
    }
    return -1;
}

bool
SnicChannel::IsBusy()
{
    if (m_state == IDLE)
    {
        return false;
    }
    else
    {
        return true;
    }
}

DataRate
SnicChannel::GetDataRate()
{
    return m_bps;
}

Time
SnicChannel::GetDelay()
{
    return m_delay;
}

// WireState
// SnicChannel::GetState()
//{
// return m_state;
//}

Ptr<NetDevice>
SnicChannel::GetDevice(std::size_t i) const
{
    return GetSnicDevice(i);
}

SnicDeviceRec::SnicDeviceRec()
{
    active = false;
}

SnicDeviceRec::SnicDeviceRec(Ptr<SnicNetDevice> device)
{
    devicePtr = device;
    active = true;
}

SnicDeviceRec::SnicDeviceRec(const SnicDeviceRec& deviceRec)
{
    devicePtr = deviceRec.devicePtr;
    active = deviceRec.active;
}

bool
SnicDeviceRec::IsActive()
{
    return active;
}

} // namespace ns3
