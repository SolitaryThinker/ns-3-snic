#include "linear-memory-modeler.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LinearMemoryModeler");
NS_OBJECT_ENSURE_REGISTERED(LinearMemoryModeler);

LinearMemoryModeler::LinearMemoryModeler(double constantMemory, double memoryPerPacket)
    : m_constantMemory(constantMemory),
      m_memoryPerPacket(memoryPerPacket)
{
}

LinearMemoryModeler::~LinearMemoryModeler()
{
}

void
LinearMemoryModeler::AddPacket(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    m_memoryUsage += m_memoryPerPacket;
}

} // namespace ns3
