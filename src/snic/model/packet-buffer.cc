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
                                          TimeValue(Seconds(1)),
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
                                          UintegerValue(3),
                                          MakeUintegerAccessor(&PacketBuffer::m_pendingQueueSize),
                                          MakeUintegerChecker<uint32_t>()) return tid;
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

void
PacketBuffer::HandleWaitReplyTimeout()
{
    NS_LOG_FUNCTION(this);
    /*
    PacketBuffer::Entry* entry;
    bool restartWaitReplyTimer = false;
    for (CacheI i = m_arpCache.begin(); i != m_arpCache.end(); i++)
    {
        entry = (*i).second;
        if (entry != nullptr && entry->IsWaitReply())
        {
            if (entry->GetRetries() < m_maxRetries)
            {
                NS_LOG_LOGIC("node=" << m_device->GetNode()->GetId() << ", ArpWaitTimeout for "
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
            }
        }
    }
    if (restartWaitReplyTimer)
    {
        NS_LOG_LOGIC("Restarting WaitReplyTimer at " << Simulator::Now().GetSeconds());
        m_waitReplyTimer =
            Simulator::Schedule(m_waitReplyTimeout, &ArpCache::HandleWaitReplyTimeout, this);
    }
    */
}

PacketBuffer::Entry::Entry(PacketBuffer* buffer)
    : m_packetBuffer(buffer),
      m_state(WAIT_REPLY),
      m_retries(0)
{
    NS_LOG_FUNCTION(this << arp);
}

void
PacketBuffer::Entry::MarkWaitReply(Ptr<const Packet> packet)
{
    // NS_LOG_FUNCTION(this << waiting.first);
    //  NS_ASSERT(m_state == ALIVE || m_state == DEAD);
    NS_ASSERT(m_pending.empty());
    // NS_ASSERT_MSG(waiting.first, "Can not add a null packet to the ARP queue");

    m_state = WAIT_REPLY;
    m_pending.push_back(waiting);
    // UpdateSeen();
    m_arp->StartWaitReplyTimer();
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

PacketBuffer::FlowId::FlowId(Ipv4Address srcIp,
                             uint16_t srcPort,
                             Ipv4Address dstIp,
                             uint16_t dstPort,
                             uint16_t protocol)
    : m_srcIp(srcIp),
      m_dstIp(dstIp),
      m_srcPort(srcPort),
      m_dstPort(dstPort),
      m_protocol(protocol)
{
}

PacketBuffer::FlowId::FlowId(const SnicHeader& snicHeader)
{
    FlowId(snicHeader.GetSourceIp(),
           snicHeader.GetSourcePort(),
           snicHeader.GetDestinationIp(),
           snicHeader.GetDestinationPort(),
           snicHeader.GetProtocol());
}

} // namespace ns3
