#ifndef SNIC_SCHEDULER_H
#define SNIC_SCHEDULER_H

#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/snic-header.h"

namespace ns3
{

class SnicScheduler : public Object
{
  public:
    static TypeId GetTypeId();

    SnicScheduler();
    ~SnicScheduler() override;
    void SetDevice(Ptr<NetDevice> device);
    Ptr<NetDevice> GetDevice() const;

    /* returns true if we are able to allocate for this flow. Fills snicHeader
     * with allocation*/
    bool Schedule(SnicHeader& snicHeader);

  private:
    Ptr<NetDevice> m_device;
    // topology table
    // active flow table
    //
};

} // namespace ns3

#endif // SNIC_SCHEDULER_H
