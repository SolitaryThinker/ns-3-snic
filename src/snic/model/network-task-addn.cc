#include "network-task-addn.h"

#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NetworkTaskAddN");

NS_OBJECT_ENSURE_REGISTERED(NetworkTaskAddN);

NetworkTaskAddN::NetworkTaskAddN()
    : m_increment(1)
{
}

NetworkTaskAddN::~NetworkTaskAddN()
{
}

void
NetworkTaskAddN::ProcessPacket(Ptr<Packet> packet)
{
    // uint32_t size = packet->GetSerializedSize();
    uint8_t buffer[8];

    packet->CopyData((uint8_t*)&buffer, 8);
    NS_LOG_INFO("buffer: " << buffer);
    buffer[7] += m_increment;
}

void
NetworkTaskAddN::SetIncrement(uint32_t increment)
{
    m_increment = increment;
}

uint32_t
NetworkTaskAddN::GetIncrement()
{
    return m_increment;
}

} // namespace ns3
