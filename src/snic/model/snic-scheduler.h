#ifndef SNIC_SCHEDULER_H
#define SNIC_SCHEDULER_H

#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/snic-scheduler-header.h"

namespace ns3
{

class SVertex
{
  public:
    enum VertexType
    {
        VertexUnknown = 0, /**< Uninitialized Link Record */
        VertexNic,         /**< Vertex representing a router in the topology */
        VertexNode         /**< Vertex representing a network in the topology */
    };

    SVertex();

    void AddVertex(SVertex* vertex);
    void SetVertexId(Ipv4Address id);
    void SetNode(Ptr<Node> node);

  private:
    VertexType m_vertexType;
    Ipv4Address m_vertexId;
    Ptr<Node> m_node;

    typedef std::list<SVertex*> ListOfSVertex_t;
    ListOfSVertex_t m_vertices;
    bool m_vertexProcessed;
};

class SnicScheduler : public Object
{
  public:
    static TypeId GetTypeId();

    SnicScheduler();
    ~SnicScheduler() override;
    void Initialize();
    void SetDevice(Ptr<NetDevice> device);
    Ptr<NetDevice> GetDevice() const;

    /* returns true if we are able to allocate for this flow. Fills snicHeader
     * with allocation*/
    bool Schedule(SnicSchedulerHeader& snicHeader);
    void Release(SnicSchedulerHeader& snicHeader);

  protected:
    void AddNode(Ptr<Node> node);

  private:
    Ptr<NetDevice> m_device;
    bool m_initialized;
    // topology table
    typedef std::list<SVertex*> ListOfSVertex_t;
    ListOfSVertex_t m_vertices;
    // topology
    // active flow table
    // map<FlowId, Allocation> m_activeFlows;
};

} // namespace ns3

#endif // SNIC_SCHEDULER_H
