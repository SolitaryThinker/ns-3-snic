#include "snic-scheduler.h"

#include "snic-net-device.h"

#include "ns3/channel.h"
#include "ns3/csma-module.h"
#include "ns3/loopback-net-device.h"
#include "ns3/node-list.h"

#include <queue>
#include <stack>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SnicScheduler");

NS_OBJECT_ENSURE_REGISTERED(SnicScheduler);

TypeId
SnicScheduler::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SnicScheduler").SetParent<Object>().SetGroupName("Snic");
    //.AddAttribute("WaitReplyTimeout",
    return tid;
}

SnicScheduler::SnicScheduler()
    : m_device(nullptr),
      m_initialized(false),
      m_allocationCount(0)
{
    NS_LOG_FUNCTION(this);
}

SnicScheduler::~SnicScheduler()
{
    NS_LOG_FUNCTION(this);
}

void
SnicScheduler::AddNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    if (m_addedNodes.count(node) == 1)
    {
        NS_LOG_DEBUG("SKIPPING SEEARCHED NODE");
        return;
    }
    // We need to create a new vertex if we have not processed this node yet
    SVertex* v = new SVertex();
    v->SetNode(node);
    m_vertices.push_back(v);
    m_addedNodes[node] = v;

    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    Ipv4Address ip;

    uint32_t numDevices = node->GetNDevices();
    NS_LOG_DEBUG("numDevices: " << numDevices);
    for (uint32_t idx = 0; idx < numDevices; idx++)
    {
        Ptr<NetDevice> dev = node->GetDevice(idx);
        int32_t interfaceNum = ipv4->GetInterfaceForDevice(dev);
        // if (interfaceNum == -1)
        // continue;

        Ptr<SnicNetDevice> snic = DynamicCast<SnicNetDevice, NetDevice>(dev);
        Ptr<CsmaNetDevice> csma = DynamicCast<CsmaNetDevice, NetDevice>(dev);
        Ptr<LoopbackNetDevice> loop = DynamicCast<LoopbackNetDevice, NetDevice>(dev);

        NS_LOG_DEBUG("\t dev: " << dev << " " << dev->GetTypeId());
        // if (loop)
        // NS_LOG_DEBUG(" IS LOOP");
        if (!csma && !snic)
        {
            // NS_LOG_DEBUG("SKIPPING AAAAAAAAAAAAAAA");
            continue;
        }

        if (snic)
        {
            NS_LOG_DEBUG("\t snic: " << snic << " " << snic->GetTypeId());
            NS_LOG_DEBUG("\t interface num: " << interfaceNum);
            Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress(interfaceNum, 0);
            ip = interfaceAddress.GetAddress();
            NS_LOG_DEBUG("\t interface addr: " << interfaceAddress);
            v->SetVertexId(ip);
            v->SetVertexType(SVertex::VertexTypeNic);
            m_nicVertices.push_back(v);
        }
        else if (csma)
        {
            NS_LOG_DEBUG("\t csma: " << csma << " " << csma->GetTypeId());
            NS_LOG_DEBUG("\t interface num: " << interfaceNum);
            if (interfaceNum > -1)
            {
                Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress(interfaceNum, 0);
                ip = interfaceAddress.GetAddress();
                NS_LOG_DEBUG("\t interface addr: " << interfaceAddress);
                v->SetVertexId(ip);
            }

            // now we go through all nodes attached
            Ptr<Channel> channel = csma->GetChannel();
            Ptr<CsmaChannel> csmaChannel = DynamicCast<CsmaChannel, Channel>(channel);

            for (std::size_t c = 0; c < csmaChannel->GetNDevices(); ++c)
            {
                Ptr<NetDevice> d = csmaChannel->GetDevice(c);
                Ptr<Node> nextNode = d->GetNode();
                NS_LOG_DEBUG("\t\t csma node: " << nextNode);
                AddNode(nextNode);
                v->AddVertex(m_addedNodes[nextNode]);
            }

            // see if we are attached to other csma netdevs
        }
    }
    if (v->GetVertexType() == SVertex::VertexTypeUnknown)
    {
        v->SetVertexType(SVertex::VertexTypeHost);
        m_hostVertices.push_back(v);
    }
}

void
SnicScheduler::PopulateStaticRoutes()
{
    NS_LOG_FUNCTION_NOARGS();

    // for (ListOfSVertex_t::iterator s_it = m_nicVertices.begin(); s_it != m_nicVertices.end();
    //++s_it)
    for (uint32_t srcIdx = 0; srcIdx < m_nicVertices.size(); srcIdx++)
    {
        SVertex* src = m_nicVertices[srcIdx];
        if (m_allPaths.count(src) == 0)
            m_allPaths[src] = std::map<SVertex*, std::vector<Path_t>>();
        // for (ListOfSVertex_t::iterator d_it = m_nicVertices.begin(); d_it != m_nicVertices.end();
        //++d_it)
        for (uint32_t dstIdx = srcIdx; dstIdx < m_nicVertices.size(); dstIdx++)
        {
            // if ((*d_it)->GetVertexType() == SVertex::VertexTypeHost)
            // continue;
            if (srcIdx != dstIdx)
            {
                SVertex* dst = m_nicVertices[dstIdx];
                NS_LOG_DEBUG("find routes between: ");
                NS_LOG_DEBUG(*src);
                NS_LOG_DEBUG(*dst);
                // XXX
                DepthFirstTraversal(src, dst, 10);
            }
        }
    }
}

void
SnicScheduler::Initialize()
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(!m_initialized);
    m_initialized = true;

    AddNode(*NodeList::Begin());

    NS_LOG_DEBUG("m_addedNodes size: " << m_addedNodes.size());

    // get all nodes
    int c = 0;
    NodeList::Iterator listEnd = NodeList::End();
    for (NodeList::Iterator i = NodeList::Begin(); i != listEnd; i++)
    {
        c++;
        Ptr<Node> node = *i;
        NS_LOG_DEBUG("#########node: " << node);
        // Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        //  uint32_t numInterfaces = ipv4->GetNInterfaces();

        // Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();
    }

    NS_LOG_DEBUG("\n=====all vertices: ");
    for (ListOfSVertex_t::iterator i = m_vertices.begin(); i != m_vertices.end(); i++)
    {
        NS_LOG_DEBUG("\tvertex: " << **i);
    }

    NS_LOG_DEBUG("\n=====all nic vertices: ");
    for (ListOfSVertex_t::iterator i = m_nicVertices.begin(); i != m_nicVertices.end(); i++)
    {
        NS_LOG_DEBUG("\tvertex: " << **i);
    }

    NS_LOG_DEBUG("\n=====all host vertices: ");
    for (ListOfSVertex_t::iterator i = m_hostVertices.begin(); i != m_hostVertices.end(); i++)
    {
        NS_LOG_DEBUG("\tvertex: " << **i);
    }

    NS_LOG_DEBUG("num node: " << c);
    // build graph
    // NS_ASSERT_MSG(m_vertices.size() == NodeList::GetNNodes(), "didnt get all the nodes");

    // NS_FATAL_ERROR("done init");
    PopulateStaticRoutes();

}

void
SnicScheduler::SetDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    m_device = device;
}

Ptr<NetDevice>
SnicScheduler::GetDevice() const
{
    NS_LOG_FUNCTION(this);
    return m_device;
}

void
SnicScheduler::DepthFirstTraversal(SVertex* src, SVertex* dst, uint32_t limit)
{
    NS_LOG_FUNCTION(this << src << dst << limit);

    std::map<SVertex*, int> visited;
    SVertex* root = src;

    NS_ASSERT(src->GetVertexType() == SVertex::VertexTypeNic);
    NS_ASSERT(dst->GetVertexType() == SVertex::VertexTypeNic);

    // std::vector<Path_t>& paths = allPaths[src][dst];
    typedef std::pair<SVertex*, std::vector<SVertex*>> s_t;

    std::stack<s_t> s;

    // visited[root] = 1;
    std::vector<SVertex*> t;
    t.push_back(root);
    s.push(std::make_pair(root, t));
    // int length = 0;
    //  Path_t path;

    // path.push(root);

    while (!s.empty())
    {
        SVertex* v = s.top().first;
        std::vector<SVertex*> p = s.top().second;

        s.pop();
        if (v == dst)
        {
            NS_LOG_DEBUG("found");
            for (uint32_t i = 0; i < p.size(); ++i)
            {
                NS_LOG_DEBUG(*p[i]);
            }
            auto reverse_path = p;
            std::reverse(reverse_path.begin(), reverse_path.end());

            m_allPaths[dst][src].push_back(reverse_path);
            m_allPaths[src][dst].push_back(p);
            // NS_LOG_DEBUG("done with printing");
            // return;
            continue;
        }
        if (visited.count(v) == 1)
        {
            continue;
        }
        visited[v] = 1;

        // NS_LOG_DEBUG("finding neighbors for: " << *v);
        for (SVertex::ListOfSVertex_t::iterator it = v->m_vertices.begin();
             it != v->m_vertices.end();
             ++it)
        {
            SVertex* neighbor = *it;
            if (neighbor->GetVertexType() == SVertex::VertexTypeHost)
            {
                continue;
            }
            // NS_LOG_DEBUG("pushing: " << *neighbor);
            auto copy = p;
            copy.push_back(neighbor);
            s.push(std::make_pair(neighbor, copy));
        }
    }
}

/* returns true if we are able to allocate for this flow. Fills snicHeader
 * with allocation*/
bool
SnicScheduler::Schedule(SnicSchedulerHeader& snicHeader)
{
    NS_LOG_FUNCTION(this << snicHeader);
    NS_LOG_DEBUG("IN SCHEDULER");
    if (!m_initialized)
    {
        Initialize();
        DumpAllPaths();
    }
    m_allocationCount++;
    NS_LOG_DEBUG("allocationCount: " << m_allocationCount);

    // while (!q.empty())
    //{
    // SVertex* v = q.dequeue();
    // if (v == g)
    //}

    // std::vector<std::vector<int>>
    //  we compute all paths between each pair
    //  m_node->GetDevice
    //   uint32_t bandwidthRequested;
    //   Ipv4Address Destination;
    /*
     *
     * for each node there are a set of devices connected. we follow that until
     * we reach snic or
     */
    return false;
}

void
SnicScheduler::Release(SnicSchedulerHeader& snicHeader)
{
}

uint64_t
SnicScheduler::GetAlllocationCount() const
{
    return m_allocationCount;
}

void
SnicScheduler::DumpAllPaths() const
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Dumping all paths: ");
    for (auto src_it = m_allPaths.begin(); src_it != m_allPaths.end(); ++src_it)
    {
        NS_LOG_DEBUG("\tSrc: \n" << *src_it->first);
        const auto& dsts = src_it->second;
        for (auto dst_it = dsts.begin(); dst_it != dsts.end(); ++dst_it)
        {
            NS_LOG_DEBUG("\tDst: \n" << *dst_it->first);
            const auto& paths = dst_it->second;
            for (auto path_it = paths.begin(); path_it != paths.end(); ++path_it)
            {
                DumpPath(*path_it);
            }
        }
    }
}

void
SnicScheduler::DumpPath(const std::vector<SVertex*>& path) const
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("path: ");
    for (uint32_t i = 0; i < path.size(); ++i)
    {
        NS_LOG_DEBUG("\t\t" << *path[i]);
    }
}

// ---------------------------------------------------------------------------
//
// SVertex Implementation
//
// ---------------------------------------------------------------------------
SVertex::SVertex()
    : m_vertexType(VertexTypeUnknown),
      m_vertexId("255.255.255.255"),
      // m_lsa(nullptr),
      // m_distanceFromRoot(SPF_INFINITY),
      // m_rootOif(SPF_INFINITY),
      // m_nextHop("0.0.0.0"),
      // m_parents(),
      // m_children(),
      m_vertices(),
      m_vertexProcessed(false)
{
    NS_LOG_FUNCTION(this);
}

void
SVertex::AddVertex(SVertex* vertex)
{
    if (vertex != this)
        m_vertices.push_back(vertex);
}

void
SVertex::SetVertexId(Ipv4Address id)
{
    m_vertexId = id;
}

void
SVertex::SetNode(Ptr<Node> node)
{
    m_node = node;
}

void
SVertex::SetVertexType(SVertex::VertexType type)
{
    m_vertexType = type;
}

Ipv4Address
SVertex::GetVertexId() const
{
    return m_vertexId;
}

Ptr<Node>
SVertex::GetNode() const
{
    return m_node;
}

int
SVertex::GetVertexType() const
{
    return m_vertexType;
}

std::ostream&
operator<<(std::ostream& os, const SVertex& vertex)
{
    os << "m_node=" << vertex.GetNode() << " (" << vertex.GetNode()->GetId() << ")"
       << "; m_vertexId=" << vertex.GetVertexId() << "; m_vertexType=" << vertex.GetVertexType()
       << "\n";

    os << "Connected: " << vertex.m_vertices.size() << "\n";
    for (SVertex::ListOfSVertex_t::const_iterator it = vertex.m_vertices.begin();
         it != vertex.m_vertices.end();
         ++it)
    {
        os << "\tm_node=" << (*it)->GetNode() << " (" << (*it)->GetNode()->GetId() << ")"
           << "\n";
    }
    os << "===========";

    return os;
}
} // namespace ns3
