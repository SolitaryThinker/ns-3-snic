#include "packet-buffer.h"

#include "ns3/address.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("PacketBuffer");

NS_OBJECT_ENSURE_REGISTERED(PacketBuffer);

TypeId
PacketBuffer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PacketBuffer")
                            .SetParent<Object>()
                            .SetGroupName("Snic")
                            .AddAttribute("WaitReplyTimeout",
                                          "When this timeout expires, "
                                          "the buffer entries will be scanned and "
                                          "entries in WaitReply state will resend ArpRequest "
                                          "unless MaxRetries has been exceeded, "
                                          "in which case the entry is marked dead",
                                          TimeValue(Seconds(5)),
                                          MakeTimeAccessor(&PacketBuffer::m_waitReplyTimeout),
                                          MakeTimeChecker())
                            .AddAttribute("MaxRetries",
                                          "Number of retransmissions of ArpRequest "
                                          "before marking dead",
                                          UintegerValue(3),
                                          MakeUintegerAccessor(&PacketBuffer::m_maxRetries),
                                          MakeUintegerChecker<uint32_t>())
                            .AddAttribute("PendingQueueSize",
                                          "The size of the queue for packets pending an arp reply.",
                                          UintegerValue(1900),
                                          MakeUintegerAccessor(&PacketBuffer::m_pendingQueueSize),
                                          MakeUintegerChecker<uint32_t>());
    return tid;
}

PacketBuffer::PacketBuffer()
    : m_device(nullptr)
{
    NS_LOG_FUNCTION(this);
}

PacketBuffer::~PacketBuffer()
{
    NS_LOG_FUNCTION(this);
}

void
PacketBuffer::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_device = nullptr;
    if (!m_waitReplyTimer.IsRunning())
    {
        m_waitReplyTimer.Cancel();
    }
    Object::DoDispose();
}

void
PacketBuffer::SetDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    m_device = device;
    // m_interface = interface;
}

Ptr<NetDevice>
PacketBuffer::GetDevice() const
{
    NS_LOG_FUNCTION(this);
    return m_device;
}

void
PacketBuffer::SetWaitReplyTimeout(Time waitReplyTimeout)
{
    NS_LOG_FUNCTION(this << waitReplyTimeout);
    m_waitReplyTimeout = waitReplyTimeout;
}

Time
PacketBuffer::GetWaitReplyTimeout() const
{
    NS_LOG_FUNCTION(this);
    return m_waitReplyTimeout;
}

void
PacketBuffer::StartWaitReplyTimer()
{
    NS_LOG_FUNCTION(this);
    if (!m_waitReplyTimer.IsRunning())
    {
        NS_LOG_LOGIC("Starting WaitReplyTimer at " << Simulator::Now() << " for "
                                                   << m_waitReplyTimeout);
        m_waitReplyTimer =
            Simulator::Schedule(m_waitReplyTimeout, &PacketBuffer::HandleWaitReplyTimeout, this);
    }
}

PacketBuffer::Entry*
PacketBuffer::Add(const FlowId& flowId)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_packetBuffer.find(flowId) == m_packetBuffer.end());

    PacketBuffer::Entry* entry = new PacketBuffer::Entry(this);
    m_packetBuffer[flowId] = entry;
    return entry;
}

void
PacketBuffer::Delete(const FlowId& flowId)
{
    NS_LOG_FUNCTION(this);
    // NS_ASSERT(m_packetBuffer.find(flowId) == m_packetBuffer.end());
    //  NS_ASSERT_MSG(m_pending.empty(), "trying to delete an entry with remaining pending
    //  packets");
    NS_ASSERT_MSG(m_packetBuffer.count(flowId) > 0,
                  "trying to delete an entry that does not exist");

    m_packetBuffer.erase(flowId);
}

PacketBuffer::Entry*
PacketBuffer::Lookup(const FlowId& flowId)
{
    NS_LOG_FUNCTION(this);

    BufferI it = m_packetBuffer.find(flowId);
    if (it != m_packetBuffer.end())
    {
        return it->second;
    }
    return nullptr;
}

void
PacketBuffer::HandleWaitReplyTimeout()
{
    NS_LOG_FUNCTION(this);

    PacketBuffer::Entry* entry;
    bool restartWaitReplyTimer = false;
    for (BufferI i = m_packetBuffer.begin(); i != m_packetBuffer.end(); i++)
    {
        entry = (*i).second;
        if (entry != nullptr && entry->IsWaitReply())
        {
            NS_FATAL_ERROR("timed OUT");
            /*
              if (entry->GetRetries() < m_maxRetries)
              {
                  NS_LOG_LOGIC("node=" << m_device->GetNode()->GetId() << ", BufferWaitTimeout for "
                                       << entry->GetIpv4Address()
                                       << " expired -- retransmitting arp request since retries = "
                                       << entry->GetRetries());
                  m_arpRequestCallback(this, entry->GetIpv4Address());
                  restartWaitReplyTimer = true;
                  entry->IncrementRetries();
              }
              else
              {
                  NS_LOG_LOGIC("node=" << m_device->GetNode()->GetId() << ", wait reply for "
                                       << entry->GetIpv4Address()
                                       << " expired -- drop since max retries exceeded: "
                                       << entry->GetRetries());
                  entry->MarkDead();
                  entry->ClearRetries();
                  Ipv4PayloadHeaderPair pending = entry->DequeuePending();
                  while (pending.first)
                  {
                      // add the Ipv4 header for tracing purposes
                      pending.first->AddHeader(pending.second);
                      m_dropTrace(pending.first);
                      pending = entry->DequeuePending();
                  }
              }*/
        }
    }
    if (restartWaitReplyTimer)
    {
        NS_LOG_LOGIC("Restarting WaitReplyTimer at " << Simulator::Now().GetSeconds());
        m_waitReplyTimer =
            Simulator::Schedule(m_waitReplyTimeout, &PacketBuffer::HandleWaitReplyTimeout, this);
    }
}

PacketBuffer::Entry::Entry(PacketBuffer* buffer)
    : m_packetBuffer(buffer),
      m_state(WAIT_REPLY),
      m_retries(0)
{
    NS_LOG_FUNCTION(this << buffer);
}

void
PacketBuffer::Entry::EnqueuePending(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    NS_ASSERT(m_state == WAIT_REPLY);
    m_pending.push_back(packet);
}

Ptr<Packet>
PacketBuffer::Entry::DequeuePending()
{
    NS_LOG_FUNCTION_NOARGS();
    if (m_pending.empty())
    {
        return nullptr;
    }
    else
    {
        Ptr<Packet> pending = m_pending.front();
        m_pending.pop_front();
        return pending;
    }
}

void
PacketBuffer::Entry::MarkActive()
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(m_state == WAIT_REPLY);
    m_state = ACTIVE;
    // add allocation
}

void
PacketBuffer::Entry::MarkWaitReply()
{
    NS_LOG_FUNCTION_NOARGS();
    // NS_LOG_FUNCTION(this << waiting.first);
    //  NS_ASSERT(m_state == ALIVE || m_state == DEAD);
    NS_ASSERT(m_pending.empty());
    // NS_ASSERT_MSG(waiting.first, "Can not add a null packet to the ARP queue");

    m_state = WAIT_REPLY;
    // UpdateSeen();
    m_packetBuffer->StartWaitReplyTimer();
}

bool
PacketBuffer::Entry::IsDone() const
{
    NS_LOG_FUNCTION(this);
    return (m_state == DONE);
}

bool
PacketBuffer::Entry::IsActive() const
{
    NS_LOG_FUNCTION(this);
    return (m_state == ACTIVE);
}

bool
PacketBuffer::Entry::IsWaitReply() const
{
    NS_LOG_FUNCTION(this);
    return (m_state == WAIT_REPLY);
}

bool
PacketBuffer::Entry::IsExpired() const
{
    NS_LOG_FUNCTION(this);
    return (m_state == EXPIRED);
}

void
PacketBuffer::Entry::SetIncomingPort(Ptr<NetDevice> incomingPort)
{
    m_incomingPort = incomingPort;
}

void
PacketBuffer::Entry::SetProtocol(uint16_t protocol)
{
    m_protocol = protocol;
}

void
PacketBuffer::Entry::SetSrc(const Address src)
{
    m_src = src;
}

void
PacketBuffer::Entry::SetDst(const Address dst)
{
    m_dst = dst;
}

Ptr<NetDevice>
PacketBuffer::Entry::GetIncomingPort() const
{
    return m_incomingPort;
}

uint16_t
PacketBuffer::Entry::GetProtocol() const
{
    return m_protocol;
}

Address
PacketBuffer::Entry::GetSrc() const
{
    return m_src;
}

Address
PacketBuffer::Entry::GetDst() const
{
    return m_dst;
}

void
PacketBuffer::Entry::SetRoute(std::list<SnicRte> route)
{
    m_rteList = route;
}

std::list<SnicRte>
PacketBuffer::Entry::GetRoute() const
{
    return m_rteList;
}

} // namespace ns3
