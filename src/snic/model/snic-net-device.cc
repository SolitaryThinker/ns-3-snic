/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#include "snic-net-device.h"

#include "network-task.h"

#include "ns3/boolean.h"
#include "ns3/flow.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/snic-header.h"
#include "ns3/snic-l4-protocol.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("SnicNetDevice");

NS_OBJECT_ENSURE_REGISTERED(SnicNetDevice);

TypeId
SnicNetDevice::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SnicNetDevice")
            .SetParent<NetDevice>()
            .SetGroupName("Snic")
            .AddConstructor<SnicNetDevice>()
            .AddAttribute("Mtu",
                          "The MAC-level Maximum Transmission Unit",
                          UintegerValue(DEFAULT_MTU),
                          MakeUintegerAccessor(&SnicNetDevice::SetMtu, &SnicNetDevice::GetMtu),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("EnableLearning",
                          "Enable the learning mode of the Learning Bridge",
                          BooleanValue(true),
                          MakeBooleanAccessor(&SnicNetDevice::m_enableLearning),
                          MakeBooleanChecker())
            .AddAttribute("ExpirationTime",
                          "Time it takes for learned MAC state entry to expire.",
                          TimeValue(Seconds(300)),
                          MakeTimeAccessor(&SnicNetDevice::m_expirationTime),
                          MakeTimeChecker());
    //.AddAttribute("InterframeGap",
    //"The time to wait between packet (frame) transmissions",
    // TimeValue(Seconds(0.0)),
    // MakeTimeAccessor(&SnicNetDevice::m_tInterframeGap),
    // MakeTimeChecker())

    return tid;
}

SnicNetDevice::SnicNetDevice()
    : m_node(nullptr),
      m_ifIndex(0),
      m_mtu(0xffff),
      m_isScheduler(false)
{
    NS_LOG_FUNCTION_NOARGS();
    m_channel = CreateObject<BridgeChannel>();

    // time_init(); // OFSI's clock; needed to use the buffer storage system.
}

SnicNetDevice::~SnicNetDevice()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
SnicNetDevice::AddSnicPort(Ptr<NetDevice> snicPort)
{
    AddSnicPort(snicPort, false);
}

void
SnicNetDevice::AddSnicPort(Ptr<NetDevice> snicPort, bool isPeerSnic)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(snicPort != this);
    NS_LOG_UNCOND("adding snic port");

    if (!Mac48Address::IsMatchingType(snicPort->GetAddress()))
    {
        NS_FATAL_ERROR("Device does not support eui 48 addresses: cannot be added to bridge.");
    }
    if (!snicPort->SupportsSendFrom())
    {
        NS_FATAL_ERROR("Device does not support SendFrom: cannot be added to bridge.");
    }
    if (m_address == Mac48Address())
    {
        m_address = Mac48Address::ConvertFrom(snicPort->GetAddress());
    }
    if (isPeerSnic)
        m_connectedSnics.push_back(snicPort->GetAddress());
    else
        m_connectedHosts.push_back(snicPort->GetAddress());

    NS_LOG_DEBUG("RegisterProtocolHandler for " << snicPort->GetInstanceTypeId().GetName());
    m_node->RegisterProtocolHandler(MakeCallback(&SnicNetDevice::ReceiveFromDevice, this),
                                    0,
                                    snicPort,
                                    true);
    m_ports.push_back(snicPort);
    NS_LOG_UNCOND("adding channel");
    m_channel->AddChannel(snicPort->GetChannel());
    NS_LOG_UNCOND("after adding channel");
}

/*
void
SnicNetDevice::AddPeerSnic(Ptr<SnicNetDevice> peerSnic, Ptr<SnicChannel> ch)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(peerSnic != this);
    NS_LOG_UNCOND("adding peer snic");

    if (!Mac48Address::IsMatchingType(peerSnic->GetAddress()))
    {
        NS_FATAL_ERROR("Device does not support eui 48 addresses: cannot be added to bridge.");
    }
    if (!peerSnic->SupportsSendFrom())
    {
        NS_FATAL_ERROR("Device does not support SendFrom: cannot be added to bridge.");
    }
    if (m_address == Mac48Address())
    {
        m_address = Mac48Address::ConvertFrom(peerSnic->GetAddress());
    }

    NS_LOG_DEBUG("RegisterProtocolHandler for " << peerSnic->GetInstanceTypeId().GetName());
    m_node->RegisterProtocolHandler(MakeCallback(&SnicNetDevice::ReceiveFromDevice, this),
                                    0,
                                    peerSnic,
                                    true);

    NS_LOG_UNCOND("adding channel");

    m_ports.push_back(peerSnic);
    m_channel->AddChannel(ch);
    ch->Attach(this);
    NS_LOG_UNCOND("after adding channel");
}
*/
// bool
// SnicNetDevice::Attach(Ptr<SnicChannel> ch)
//{
// }

uint32_t
SnicNetDevice::GetNSnicPorts() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_ports.size();
}

Ptr<NetDevice>
SnicNetDevice::GetSnicPort(uint32_t n) const
{
    NS_LOG_FUNCTION(this << n);
    return m_ports[n];
}

void
SnicNetDevice::AddNT(Ptr<NetworkTask> nt, uint32_t id)
{
    NS_LOG_FUNCTION(this << nt << id);
    m_nts[id] = nt;
}

void
SnicNetDevice::RemoveNT(uint32_t id)
{
    NS_LOG_FUNCTION(this << id);
    m_nts.erase(id);
}

Ptr<NetworkTask>
SnicNetDevice::GetNT(uint32_t id)
{
    NS_LOG_FUNCTION(this << id);
    return m_nts[id];
}

uint32_t
SnicNetDevice::GetNumNT()
{
    NS_LOG_FUNCTION_NOARGS();
    return m_nts.size();
}

void
SnicNetDevice::RequestAllocation(Ptr<const Packet> packet, uint16_t protocol)
{
    NS_LOG_FUNCTION_NOARGS();
    Ptr<Packet> copy = packet->Copy();

    Ptr<Packet> request = Create<Packet>();
    Ipv4Header ipv4Header;
    SnicHeader snicHeader;
    copy->RemoveHeader(ipv4Header);
    copy->RemoveHeader(snicHeader);
    SnicSchedulerHeader schedHeader = SnicSchedulerHeader(ipv4Header);

    schedHeader.SetBandwidthDemand(10);
    schedHeader.SetResourceDemand(5);
    schedHeader.AddNT(5);
    schedHeader.SetPacketType(SnicSchedulerHeader::ALLOCATION_REQUEST);

    ipv4Header.SetDestination(m_schedulerAddress);
    ipv4Header.SetSource(m_ipAddress);

    snicHeader.SetPacketType(SnicHeader::ALLOCATION_REQUEST);

    request->AddHeader(schedHeader);
    request->AddHeader(snicHeader);
    request->AddHeader(ipv4Header);
    // create new flow to track packets
    FlowId flowId = FlowId(schedHeader);
    // make the flow known to the packet cache
    PacketBuffer::Entry* entry = m_packetBuffer.Add(flowId);
    m_packetBuffer.SetWaitReplyTimeout(Seconds(5));
    entry->MarkWaitReply();
    entry->EnqueuePending(packet);
    //  newentry = flowid;
    //  m_packetBuffer.EnqueuePending(flowid, packet);
    //  m_packetBuffer.EnqueuePending(flowid, packet);
    m_rxCallback(this, request, protocol, m_address);
}

void
SnicNetDevice::SetSchedulerAddress(Ipv4Address schedulerAddress)
{
    NS_LOG_FUNCTION(this << schedulerAddress);
    m_schedulerAddress = schedulerAddress;
}

Ipv4Address
SnicNetDevice::GetSchedulerAddress() const
{
    return m_schedulerAddress;
}

void
SnicNetDevice::SetIsScheduler(bool isScheduler)
{
    NS_LOG_FUNCTION(this << isScheduler);
    m_isScheduler = isScheduler;
}

bool
SnicNetDevice::IsScheduler() const
{
    return m_isScheduler;
}

void
SnicNetDevice::SetIpAddress(Ipv4Address address)
{
    m_ipAddress = address;
}

Ipv4Address
SnicNetDevice::GetIpAddress() const
{
    return m_ipAddress;
}

void
SnicNetDevice::DoDispose()
{
    NS_LOG_FUNCTION(this);
    for (std::vector<Ptr<NetDevice>>::iterator iter = m_ports.begin(); iter != m_ports.end();
         iter++)
    {
        *iter = nullptr;
    }
    m_node = nullptr;
    m_channel = nullptr;
    m_currentPkt = nullptr;
    NetDevice::DoDispose();
}

void
SnicNetDevice::HandleIpv4Packet(Ptr<NetDevice> incomingPort,
                                Ptr<Packet> packet,
                                uint16_t protocol,
                                const Address& src,
                                const Address& dst)
{
    NS_LOG_FUNCTION(this << incomingPort << protocol);

    Mac48Address src48 = Mac48Address::ConvertFrom(src);
    Mac48Address dst48 = Mac48Address::ConvertFrom(dst);
    bool addressedToUs = (dst48 == m_address);

    Ipv4Header ipv4Header;
    SnicHeader snicHeader;
    packet->RemoveHeader(ipv4Header);
    packet->RemoveHeader(snicHeader);
    NS_LOG_DEBUG("\tseen nic?: " << snicHeader.HasSeenNic());
    NS_LOG_DEBUG("\tis Scheduler?: " << IsScheduler());
    // packet->AddHeader(

    switch (snicHeader.GetPacketType())
    {
    case SnicHeader::L4_PACKET: {
        if (addressedToUs)
        {
            NS_FATAL_ERROR("l4 packets shouldn't be addressed to snics");
        }
        else
        {
            if (snicHeader.HasSeenNic())
            {
                NS_LOG_DEBUG("FOWARDING");

                packet->AddHeader(snicHeader);
                packet->AddHeader(ipv4Header);
                ForwardUnicast(incomingPort, packet, protocol, src48, dst48);
                return;
            }
            snicHeader.SetHasSeenNic();
            if (IsScheduler())
            {
                NS_LOG_DEBUG("sched without alloc request NOW============");
                // ForwardBroadcast(
                // scheduler
            }
            else
            {
                NS_LOG_DEBUG("req sched ============");
                packet->AddHeader(snicHeader);
                packet->AddHeader(ipv4Header);
                RequestAllocation(packet, protocol);
                return;
            }
        }
        break;
    }
    case SnicHeader::ALLOCATION_REQUEST: {
        // if the packet is an allocation request and not addressed for us, we
        // forward it to the scheduler
        if (!addressedToUs)
        {
            NS_LOG_DEBUG("req not addressed to us");
            packet->AddHeader(snicHeader);
            packet->AddHeader(ipv4Header);
            ForwardUnicast(incomingPort, packet, protocol, src48, dst48);
            return;
        }
        SnicSchedulerHeader schedHeader;
        packet->RemoveHeader(schedHeader);

        // if we aren't the scheduler then something went wrong
        NS_ASSERT_MSG(IsScheduler(), "packet is addressed to us but we are not scheduler");
        // if (!IsScheduler())
        //{
        // NS_FATAL_ERROR("");
        //}
        NS_LOG_DEBUG("running sched");
        m_scheduler.Schedule(schedHeader);
        //  reply
        Ptr<Packet> response = Create<Packet>();
        SnicSchedulerHeader responseSchedHeader(ipv4Header);
        SnicHeader responseSnicHeader(snicHeader);
        Ipv4Header responseIpv4Header(ipv4Header);
        responseSnicHeader.SetPacketType(SnicHeader::ALLOCATION_RESPONSE);

        responseSnicHeader.SetSourcePort(snicHeader.GetDestinationPort());
        responseSnicHeader.SetDestinationPort(snicHeader.GetSourcePort());
        responseSnicHeader.SetSourceIp(snicHeader.GetDestinationIp());
        responseSnicHeader.SetDestinationIp(snicHeader.GetSourceIp());

        responseIpv4Header.SetSource(ipv4Header.GetDestination());
        responseIpv4Header.SetDestination(ipv4Header.GetSource());
        NS_LOG_DEBUG("creating response");
        NS_LOG_DEBUG("old src: " << ipv4Header.GetSource());
        NS_LOG_DEBUG("old dest: " << ipv4Header.GetDestination());

        response->AddHeader(responseSchedHeader);
        response->AddHeader(responseSnicHeader);
        response->AddHeader(responseIpv4Header);
        m_rxCallback(this, response, protocol, dst);
        break;
    }
    case SnicHeader::ALLOCATION_RESPONSE: {
        if (!addressedToUs)
        {
            packet->AddHeader(snicHeader);
            packet->AddHeader(ipv4Header);
            ForwardUnicast(incomingPort, packet, protocol, src48, dst48);
            return;
        }
        SnicSchedulerHeader schedHeader;
        packet->RemoveHeader(schedHeader);
        NS_LOG_DEBUG("got response");
        // get flowid from response packet
        FlowId flowId = FlowId(schedHeader);
        // find entry in packet buffer
        PacketBuffer::Entry* entry = m_packetBuffer.Lookup(flowId);
        entry->MarkActive();
        // for (... send pending packets)

        // markactive
        // dequeue pending packets
        // send all pending packets to destination using received route.
        break;
    }
    case SnicHeader::ALLOCATION_RELEASE: {
        if (!addressedToUs)
        {
            packet->AddHeader(snicHeader);
            packet->AddHeader(ipv4Header);
            ForwardUnicast(incomingPort, packet, protocol, src48, dst48);
            return;
        }
        NS_LOG_DEBUG("got release");
        break;
    }
    default:
        NS_FATAL_ERROR("unhandled snic packet type");
    }

    // packet = ProcessPacket(incomingPort, packet, protocol, src, dst);
    // NS_LOG_DEBUG("\tsrc ipaddress: " << src);
    // NS_LOG_DEBUG("\tsrc: " << src);
    // NS_LOG_DEBUG("\tdst: " << " " << dst);
    // for (uint64_t i = 0; i < m_connectedHosts.size(); i++)
    //{
    // NS_LOG_DEBUG("connectedHost " << i << ": " <<
    // m_connectedHosts[i]); if (src == m_connectedHosts[i]) NS_LOG_DEBUG("from our
    // host");
    // }
    // for (uint64_t i = 0; i < m_connectedSnics.size(); i++)
    //{
    // NS_LOG_DEBUG("connectedSnics " << i << ": " <<
    // m_connectedSnics[i]);
    // }
    // for (uint32_t i = 0; i < m_node->GetNDevices(); i++)
    //{
    // NS_LOG_DEBUG("device " << i << ": " <<
    // m_node->GetDevice(i)->GetAddress());
    // }
    //}
}

void
SnicNetDevice::ReceiveFromDevice(Ptr<NetDevice> incomingPort,
                                 Ptr<Packet> packet,
                                 uint16_t protocol,
                                 const Address& src,
                                 const Address& dst,
                                 PacketType packetType)
{
    Packet::EnablePrinting();
    NS_LOG_FUNCTION(this << incomingPort << protocol);
    NS_LOG_DEBUG("UID is " << packet->GetUid());
    NS_LOG_DEBUG("id is " << m_node->GetId());

    Mac48Address src48 = Mac48Address::ConvertFrom(src);
    Mac48Address dst48 = Mac48Address::ConvertFrom(dst);
    NS_LOG_DEBUG("mac src is " << src48);
    NS_LOG_DEBUG("mac dest is " << dst48);
    // NS_LOG_DEBUG("src is " << InetSocketAddress::ConvertFrom(src));
    // NS_LOG_DEBUG("dest is " << InetSocketAddress::ConvertFrom(dst));
    NS_LOG_DEBUG("m_address is " << m_address);
    NS_LOG_DEBUG("packetType is " << packetType);
    SnicHeader snicHeader;
    Ptr<Packet> copypkt = packet->Copy();
    copypkt->RemoveHeader(snicHeader);
    std::ostringstream coll;

    packet->Print(coll);
    // snicHeader.Print(coll);

    NS_LOG_DEBUG("header is " << coll.str());
    NS_LOG_DEBUG("header nt is " << snicHeader.GetNT());

    if (!m_promiscRxCallback.IsNull())
    {
        NS_LOG_DEBUG("doing promisc");
        m_promiscRxCallback(this, packet, protocol, src, dst, packetType);
    }
    NS_LOG_DEBUG("after promisc");

    switch (packetType)
    {
    case PACKET_HOST:
        if (dst48 == m_address)
        {
            NS_LOG_DEBUG("packetType PACKET_HOST our address");
            // unwrap snic header
            Learn(src48, incomingPort);
            m_rxCallback(this, packet, protocol, src);
        }
        break;

    case PACKET_BROADCAST:
    case PACKET_MULTICAST:
        NS_LOG_DEBUG("packetType PACKET_MULTICAST ");
        // if (protocol != ArpL3Protocol::PROT_NUMBER)
        m_rxCallback(this, packet, protocol, src);
        // warp snic header
        ForwardBroadcast(incomingPort, packet, protocol, src48, dst48);
        break;

    case PACKET_OTHERHOST:
        NS_LOG_DEBUG("packetType PACKET_OTHERHOST");

        switch (protocol)
        {
        case IPV4_PROT_NUMBER: {
            NS_LOG_DEBUG("protocol Ipv4L3Protocol");
            HandleIpv4Packet(incomingPort, packet, protocol, src48, dst48);
            break;
        }
        default:
            NS_LOG_DEBUG("protocol other");
            if (dst48 == m_address)
            {
                NS_LOG_DEBUG("match our address ");
                // NS_FATAL_ERROR("packet is addressed to us but we are not sched");
                Learn(src48, incomingPort);
                m_rxCallback(this, packet, protocol, src);
            }
            else
            {
                NS_LOG_DEBUG("not our address ");
                ForwardUnicast(incomingPort, packet, protocol, src48, dst48);
            }
            break;
        }
        break;
    }
    NS_LOG_DEBUG("Done================");
}

void
SnicNetDevice::Receive(Ptr<Packet> p, Ptr<SnicNetDevice> sender)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_UNCOND("==========Receieved from peer Snic=======");
}

Ptr<Packet>
SnicNetDevice::ProcessPacket(Ptr<NetDevice> incomingPort,
                             Ptr<const Packet> packet,
                             uint16_t protocol,
                             const Address& src,
                             const Address& dst)
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> pktCopy = packet->Copy();

    Ipv4Header ipv4Header;
    // strip ipv4 header
    pktCopy->RemoveHeader(ipv4Header);

    SnicHeader snicHeader;
    // strip snic header
    pktCopy->RemoveHeader(snicHeader);

    // process snic header
    // TODO check for valid nt id
    m_nts[snicHeader.GetNT()]->ProcessHeader(snicHeader);

    // add snic header
    pktCopy->AddHeader(snicHeader);
    // add ipv4 header
    pktCopy->AddHeader(ipv4Header);

    // parse
    // match
    // action
    // extract header,
    // figure out what NT/ protocols are needed
    // if we need to offload
    // how to route
    return pktCopy;
}

void
SnicNetDevice::ForwardUnicast(Ptr<NetDevice> incomingPort,
                              Ptr<const Packet> packet,
                              uint16_t protocol,
                              Mac48Address src,
                              Mac48Address dst)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_DEBUG("LearningBridgeForward (incomingPort="
                 << incomingPort->GetInstanceTypeId().GetName() << ", packet=" << packet
                 << ", protocol=" << protocol << ", src=" << src << ", dst=" << dst << ")");

    Learn(src, incomingPort);
    Ptr<NetDevice> outPort = GetLearnedState(dst);
    if (outPort && outPort != incomingPort)
    {
        NS_LOG_LOGIC("Learning bridge state says to use port `"
                     << outPort->GetInstanceTypeId().GetName() << "'");
        outPort->SendFrom(packet->Copy(), src, dst, protocol);
    }
    else
    {
        NS_LOG_LOGIC("No learned state: send through all ports");
        for (std::vector<Ptr<NetDevice>>::iterator iter = m_ports.begin(); iter != m_ports.end();
             iter++)
        {
            Ptr<NetDevice> port = *iter;
            if (port != incomingPort)
            {
                NS_LOG_LOGIC("LearningBridgeForward ("
                             << src << " => " << dst
                             << "): " << incomingPort->GetInstanceTypeId().GetName() << " --> "
                             << port->GetInstanceTypeId().GetName() << " (UID " << packet->GetUid()
                             << ").");
                port->SendFrom(packet->Copy(), src, dst, protocol);
            }
        }
    }
}

void
SnicNetDevice::ForwardBroadcast(Ptr<NetDevice> incomingPort,
                                Ptr<const Packet> packet,
                                uint16_t protocol,
                                Mac48Address src,
                                Mac48Address dst)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_DEBUG("LearningSnicForward (incomingPort="
                 << incomingPort->GetInstanceTypeId().GetName() << ", packet=" << packet
                 << ", protocol=" << protocol << ", src=" << src << ", dst=" << dst << ")");
    Learn(src, incomingPort);

    for (std::vector<Ptr<NetDevice>>::iterator iter = m_ports.begin(); iter != m_ports.end();
         iter++)
    {
        Ptr<NetDevice> port = *iter;
        if (port != incomingPort)
        {
            NS_LOG_LOGIC("LearningSnicForward (" << src << " => " << dst << "): "
                                                 << incomingPort->GetInstanceTypeId().GetName()
                                                 << " --> " << port->GetInstanceTypeId().GetName()
                                                 << " (UID " << packet->GetUid() << ").");
            port->SendFrom(packet->Copy(), src, dst, protocol);
        }
    }
}

void
SnicNetDevice::Learn(Mac48Address source, Ptr<NetDevice> port)
{
    NS_LOG_FUNCTION_NOARGS();
    if (m_enableLearning)
    {
        LearnedState& state = m_learnState[source];
        state.associatedPort = port;
        state.expirationTime = Simulator::Now() + m_expirationTime;
    }
}

Ptr<NetDevice>
SnicNetDevice::GetLearnedState(Mac48Address source)
{
    NS_LOG_FUNCTION_NOARGS();
    if (m_enableLearning)
    {
        Time now = Simulator::Now();
        std::map<Mac48Address, LearnedState>::iterator iter = m_learnState.find(source);
        if (iter != m_learnState.end())
        {
            LearnedState& state = iter->second;
            if (state.expirationTime > now)
            {
                return state.associatedPort;
            }
            else
            {
                m_learnState.erase(iter);
            }
        }
    }
    return nullptr;
}

void
SnicNetDevice::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(this);
    m_ifIndex = index;
}

uint32_t
SnicNetDevice::GetIfIndex() const
{
    return m_ifIndex;
}

Ptr<Channel>
SnicNetDevice::GetChannel() const
{
    return m_channel;
}

//
// This is a point-to-point device, so we really don't need any kind of address
// information.  However, the base class NetDevice wants us to define the
// methods to get and set the address.  Rather than be rude and assert, we let
// clients get and set the address, but simply ignore them.

void
SnicNetDevice::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_address = Mac48Address::ConvertFrom(address);
}

Address
SnicNetDevice::GetAddress() const
{
    return m_address;
}

bool
SnicNetDevice::IsLinkUp() const
{
    NS_LOG_FUNCTION_NOARGS();
    return true;
}

void
SnicNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
}

//
// This is a point-to-point device, so every transmission is a broadcast to
// all of the devices on the network.
//
bool
SnicNetDevice::IsBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

//
// We don't really need any addressing information since this is a
// point-to-point device.  The base class NetDevice wants us to return a
// broadcast address, so we make up something reasonable.
//
Address
SnicNetDevice::GetBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address("ff:ff:ff:ff:ff:ff");
}

bool
SnicNetDevice::IsMulticast() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

Address
SnicNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address("01:00:5e:00:00:00");
}

Address
SnicNetDevice::GetMulticast(Ipv6Address addr) const
{
    NS_LOG_FUNCTION(this << addr);
    return Mac48Address("33:33:00:00:00:00");
}

bool
SnicNetDevice::IsPointToPoint() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

bool
SnicNetDevice::IsBridge() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
SnicNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION_NOARGS();
    return SendFrom(packet, m_address, dest, protocolNumber);
}

bool
SnicNetDevice::SendFrom(Ptr<Packet> packet,
                                const Address& source,
                                const Address& dest,
                                uint16_t protocolNumber)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_UNCOND("NOW IN OTHER NIC");

    Mac48Address dst = Mac48Address::ConvertFrom(dest);

    // try to use the learned state if data is unicast
    if (!dst.IsGroup())
    {
        Ptr<NetDevice> outPort = GetLearnedState(dst);
        if (outPort)
        {
            outPort->SendFrom(packet, source, dest, protocolNumber);
            return true;
        }
    }

    // data was not unicast or no state has been learned for that mac
    // address => flood through all ports.
    Ptr<Packet> pktCopy;
    for (std::vector<Ptr<NetDevice>>::iterator iter = m_ports.begin(); iter != m_ports.end();
         iter++)
    {
        pktCopy = packet->Copy();
        Ptr<NetDevice> port = *iter;
        port->SendFrom(pktCopy, source, dest, protocolNumber);
    }

    return true;
}

Ptr<Node>
SnicNetDevice::GetNode() const
{
    return m_node;
}

void
SnicNetDevice::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this);
    m_node = node;
}

bool
SnicNetDevice::NeedsArp() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

void
SnicNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
    m_rxCallback = cb;
}

void
SnicNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
    m_promiscRxCallback = cb;
}

bool
SnicNetDevice::SupportsSendFrom() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

bool
SnicNetDevice::SetMtu(uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    return true;
}

uint16_t
SnicNetDevice::GetMtu() const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}

/*
uint16_t
SnicNetDevice::PppToEther(uint16_t proto)
{
    NS_LOG_FUNCTION_NOARGS();
    switch (proto)
    {
    case 0x0021:
        return 0x0800; // IPv4
    case 0x0057:
        return 0x86DD; // IPv6
    default:
        NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
    }
    return 0;
}

uint16_t
SnicNetDevice::EtherToPpp(uint16_t proto)
{
    NS_LOG_FUNCTION_NOARGS();
    switch (proto)
    {
    case 0x0800:
        return 0x0021; // IPv4
    case 0x86DD:
        return 0x0057; // IPv6
    default:
        NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
    }
    return 0;
}
*/

} // namespace ns3
