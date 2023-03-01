#ifndef SNIC_SCHEDULER_H
#define SNIC_SCHEDULER_H

#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/snic-scheduler-header.h"

#include <list>
#include <map>
#include <ostream>

namespace ns3
{

class SVertex
{
  public:
    friend class SnicScheduler;
    enum VertexType
    {
        VertexTypeUnknown = 0, /**< Uninitialized Link Record */
        VertexTypeNic,         /**< Vertex representing a router in the topology */
        VertexTypeHost         /**< Vertex representing a network in the topology */
    };

    SVertex();

    void AddVertex(SVertex* vertex);
    void SetVertexId(Ipv4Address id);
    void SetNode(Ptr<Node> node);
    void SetVertexType(VertexType type);

    Ipv4Address GetVertexId() const;
    Ptr<Node> GetNode() const;
    int GetVertexType() const;

  private:
    typedef std::list<SVertex*> ListOfSVertex_t;
    friend std::ostream& operator<<(std::ostream& os, const SVertex& vertex);
    VertexType m_vertexType;
    Ipv4Address m_vertexId;
    Ptr<Node> m_node;

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

    uint64_t GetAlllocationCount() const;

  protected:
    void AddNode(Ptr<Node> node);
    void DepthFirstTraversal(SVertex* src, SVertex* dst, uint32_t limit);
    void PopulateStaticRoutes();

  private:
    Ptr<NetDevice> m_device;
    bool m_initialized;
    // topology table
    typedef std::vector<SVertex*> ListOfSVertex_t;
    ListOfSVertex_t m_vertices;
    ListOfSVertex_t m_nicVertices;
    ListOfSVertex_t m_hostVertices;
    std::map<Ptr<Node>, SVertex*> m_addedNodes;

    typedef std::vector<SVertex*> Path_t;
    std::map<SVertex*, std::map<SVertex*, std::vector<Path_t>>> allPaths;
    // topology
    // active flow table
    // map<FlowId, Allocation> m_activeFlows;
    uint64_t m_allocationCount;
};

std::ostream& operator<<(std::ostream& os, const SVertex& vertex);

} // namespace ns3

#endif // SNIC_SCHEDULER_H
