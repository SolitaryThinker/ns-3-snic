
#include "snic-scheduler.h"

#include "snic-net-device.h"

#include "ns3/csma-net-device.h"
#include "ns3/node-list.h"

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
      m_initialized(false)
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
    SVertex* v = new SVertex();

    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    Ipv4Address ip;

    uint32_t numDevices = node->GetNDevices();
    for (uint32_t idx = 0; idx < numDevices; idx++)
    {
        Ptr<NetDevice> dev = node->GetDevice(idx);
        int32_t interfaceNum = ipv4->GetInterfaceForDevice(dev);
        if (interfaceNum == -1)
            continue;

        Ptr<SnicNetDevice> snic = DynamicCast<SnicNetDevice, NetDevice>(dev);
        Ptr<CsmaNetDevice> csma = DynamicCast<CsmaNetDevice, NetDevice>(dev);

        if (!csma && !snic)
            continue;
        // dev->GetDevice

        NS_LOG_DEBUG("\t dev: " << dev << " " << dev->GetTypeId());
        if (snic)
        {
            NS_LOG_DEBUG("\t snic: " << snic << " " << snic->GetTypeId());
            NS_LOG_DEBUG("\t interface num: " << interfaceNum);
            Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress(interfaceNum, 0);
            NS_LOG_DEBUG("\t interface addr: " << interfaceAddress);
        }
        else if (csma)
        {
            NS_LOG_DEBUG("\t csma: " << csma << " " << csma->GetTypeId());
            NS_LOG_DEBUG("\t interface num: " << interfaceNum);
            Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress(interfaceNum, 0);
            NS_LOG_DEBUG("\t interface addr: " << interfaceAddress);
        }
    }
    v->SetVertexId(ip);
    v->SetNode(node);
    // for
    m_vertices.push_back(v);
}

void
SnicScheduler::Initialize()
{
    NS_LOG_FUNCTION_NOARGS();
    m_initialized = true;

    // get all nodes
    int c = 0;
    NodeList::Iterator listEnd = NodeList::End();
    for (NodeList::Iterator i = NodeList::Begin(); i != listEnd; i++)
    {
        c++;
        Ptr<Node> node = *i;
        NS_LOG_DEBUG("#########node: " << node);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        // uint32_t numInterfaces = ipv4->GetNInterfaces();

        // Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();
    }
    NS_LOG_DEBUG("num node: " << c);
    // build graph

    NS_FATAL_ERROR("done init");
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
    }
    // m_node->GetDevice
    //  uint32_t bandwidthRequested;
    //  Ipv4Address Destination;
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

// ---------------------------------------------------------------------------
//
// SVertex Implementation
//
// ---------------------------------------------------------------------------
SVertex::SVertex()
    : m_vertexType(VertexUnknown),
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

} // namespace ns3
