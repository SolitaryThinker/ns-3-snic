#include "memory-modeler.h"

namespace ns3
{

MemoryModeler::MemoryModeler()
{
}

MemoryModeler::~MemoryModeler()
{
}

double
MemoryModeler::GetMemoryUsage() const
{
    return m_memoryUsage;
}

} // namespace ns3
