#ifndef LINEAR_MEMORY_MODELER_H
#define LINEAR_MEMORY_MODELER_H

#include "ns3/memory-modeler.h"

namespace ns3
{

class LinearMemoryModeler : public MemoryModeler
{
  public:
    LinearMemoryModeler(double constantMemory, double memoryPerPacket);
    ~LinearMemoryModeler() override;

    double GetMemoryUsage() const;
    void AddPacket(Ptr<Packet> packet);

  private:
    // double m_memoryUsage;

    double m_constantMemory;

    double m_memoryPerPacket;
};
} // namespace ns3

#endif // MEMORY_MODELER_H
