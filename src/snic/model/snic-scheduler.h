#ifndef SNIC_SCHEDULER_H
#define SNIC_SCHEDULER_H

#include "ns3/csma-module.h"
#include "ns3/data-rate.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/snic-scheduler-header.h"

#include <list>
#include <map>
#include <ostream>

namespace ns3
{
class SEdge;
class SVertex
{
  public:
    friend class SnicScheduler;
    friend class SEdge;

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

    SVertex* GetConnectedVertex(uint32_t id) const;

    SEdge* GetEdgeTo(SVertex* v);

  protected:
    void AddEdge(SEdge* edge, SVertex* other);

  private:
    typedef std::vector<SVertex*> ListOfSVertex_t;
    friend std::ostream& operator<<(std::ostream& os, const SVertex& vertex);
    VertexType m_vertexType;
    Ipv4Address m_vertexId;
    Ptr<Node> m_node;

    typedef std::vector<SEdge*> ListOfSEdge_t;
    ListOfSVertex_t m_vertices;
    bool m_vertexProcessed;
    ListOfSEdge_t m_edges;
    std::map<SVertex*, SEdge*> m_edgeMap;
};

class SEdge
{
  public:
    friend class SnicScheduler;
    friend class SVertex;

    SEdge();
    void SetVertices(SVertex* left, SVertex* right);
    void SetLVertex(SVertex* v);
    void SetRVertex(SVertex* v);

    SVertex* GetLVertex() const;
    SVertex* GetRVertex() const;

    DataRate GetRemainingBandwidth() const;

    void SetChannel(Ptr<CsmaChannel> channel);

  private:
    friend std::ostream& operator<<(std::ostream& os, const SEdge& edge);
    SVertex* m_leftVertex;
    SVertex* m_rightVertex;
    Ptr<Channel> m_channel;
    DataRate m_consumedBandwidth;
    DataRate m_remainingBandwidth;

    DataRate m_totalBandwidth;
    // DataRate m_cosu
    //  FIXME
    std::map<SVertex*, double> m_allocatedBandwidth;
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
    typedef std::vector<SVertex*> Path_t;
    bool Schedule(SnicSchedulerHeader& snicHeader);
    void Release(SnicSchedulerHeader& snicHeader);

    uint64_t GetAlllocationCount() const;
    void DumpAllPaths() const;
    void DumpPath(const std::vector<SVertex*>& path) const;
    void DumpEdges() const;

  protected:

    void AddNode(Ptr<Node> node);
    void DepthFirstTraversal(SVertex* src, SVertex* dst, uint32_t limit);
    void PopulateStaticRoutes();
    void InitializeResources();

    SVertex* GetVertexFromIp(const Ipv4Address& ip) const;

    bool PathIsValid(const SnicSchedulerHeader& header, Path_t path) const;
    void AllocatePath(Path_t path);

  private:
    Ptr<NetDevice> m_device;
    bool m_initialized;
    // topology table
    typedef std::vector<SVertex*> ListOfSVertex_t;
    ListOfSVertex_t m_vertices;
    ListOfSVertex_t m_nicVertices;
    ListOfSVertex_t m_hostVertices;
    std::map<Ptr<Node>, SVertex*> m_addedNodes;

    typedef std::vector<SEdge*> ListOfSEdge_t;
    ListOfSEdge_t m_edges;

    std::map<SVertex*, std::map<SVertex*, std::vector<Path_t>>> m_allPaths;
    // topology
    // active flow table
    // map<FlowId, Allocation> m_activeFlows;
    uint64_t m_allocationCount;

    enum ResourceType
    {
        FPGA = 0,
        INGRESS,
        EGRESS,
        MEMORY
    };

    // indexed by resource
    std::map<uint8_t, double> m_resourceConsumed;
    // indexed by vertex
    std::map<SVertex*, std::map<uint8_t, double>> m_resourceAllocated;
    // indexed by resource
    std::map<uint8_t, double> m_resourceRemaining;
};

std::ostream& operator<<(std::ostream& os, const SVertex& vertex);

} // namespace ns3

#endif // SNIC_SCHEDULER_H
