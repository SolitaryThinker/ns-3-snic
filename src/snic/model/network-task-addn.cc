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
NetworkTaskAddN::ProcessHeader(SnicHeader& header)
{
    NS_LOG_FUNCTION_NOARGS();
    // uint32_t size = header->GetSerializedSize();
    uint8_t buffer[8];

    header.CopyPayload(buffer, 8);
    *(int64_t*)buffer += m_increment;
    NS_LOG_INFO("buffer: " << buffer);
    header.SetPayload(buffer, 8);
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
