#ifndef MEMORY_MODELER_H
#define MEMORY_MODELER_H

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

namespace ns3
{

class MemoryModeler : public Object
{
  public:
    MemoryModeler();
    ~MemoryModeler();

    double GetMemoryUsage() const;
    virtual void AddPacket(Ptr<Packet> packet) = 0;

  protected:
    double m_memoryUsage;

  private:
};

} // namespace ns3

#endif // MEMORY_MODELER_H
