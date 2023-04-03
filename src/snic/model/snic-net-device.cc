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
                          MakeTimeChecker())
            .AddTraceSource("SchedTrace",
                            "Number of scheduler requests made by this NIC",
                            MakeTraceSourceAccessor(&SnicNetDevice::m_schedTrace),
                            "ns3::SnicNetDevice::SchedTracedCallback")
            .AddTraceSource("NumL4Packets",
                            "Number of L4 packets seen by this NIC",
                            MakeTraceSourceAccessor(&SnicNetDevice::m_numL4Packets),
                            "ns3::TracedValueCallback::UInt64");
    //.AddAttribute("InterframeGap",
    //"The time to wait between packet (frame) transmissions",
    // TimeValue(Seconds(0.0)),
    // MakeTimeAccessor(&SnicNetDevice::m_tInterframeGap),
    // MakeTimeChecker())

    return tid;
}

SnicNetDevice::SnicNetDevice()
    : m_snicId(0),
      m_node(nullptr),
      m_ifIndex(0),
      m_mtu(0xffff),
      m_isScheduler(false),
      m_currentFlowId(0)
{
    NS_LOG_FUNCTION_NOARGS();
    m_channel = CreateObject<BridgeChannel>();

    // time_init(); // OFSI's clock; needed to use the buffer storage system.
}

SnicNetDevice::~SnicNetDevice()
{
    NS_LOG_FUNCTION_NOARGS();
}

uint32_t
SnicNetDevice::GetSnicId() const
{
    return m_snicId;
}

void
SnicNetDevice::SetSnicId(uint32_t id)
{
    m_snicId = id;
}

void
SnicNetDevice::AddSnicPort(Ptr<NetDevice> snicPort)
{
    AddSnicPort(snicPort, false);
}

void
SnicNetDevice::AddSnicPort(Ptr<NetDevice> snicPort, bool isPeerSnic)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(snicPort != this);
    NS_LOG_DEBUG("adding snic port addr: " << snicPort->GetAddress());

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
    AddAddress(Mac48Address::ConvertFrom(snicPort->GetAddress()));
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
    m_channel->AddChannel(snicPort->GetChannel());
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

uint64_t
SnicNetDevice::GetNumSchedReqs() const
{
    return m_numSchedReqs;
}

void
SnicNetDevice::AllocationRequest(Ptr<NetDevice> incomingPort,
                                 Ptr<Packet> packet,
                                 uint16_t protocol,
                                 const Address& src,
                                 const Address& dst)
{
    NS_LOG_FUNCTION_NOARGS();
    Ptr<Packet> copy = packet->Copy();

    Ptr<Packet> request = Create<Packet>();
    Ipv4Header ipv4Header;
    SnicHeader snicHeader;
    copy->RemoveHeader(ipv4Header);
    copy->RemoveHeader(snicHeader);
    SnicSchedulerHeader schedHeader;
    // NS_LOG_DEBUG("flowid in schedheader: " << schedHeader.GetFlowId());
    // NS_LOG_DEBUG("flowid in schedheader: " << schedHeader.GetDestinationIp());
    NS_ASSERT_MSG(snicHeader.IsNewFlow(), "is not new flow!");

    schedHeader.SetBandwidthDemand(snicHeader.GetTput());
    // schedHeader.SetBandwidthDemand(20.55);
    schedHeader.SetResourceDemand(5);
    schedHeader.AddNT(5);
    schedHeader.SetPacketType(SnicSchedulerHeader::ALLOCATION_REQUEST);
    schedHeader.SetFlowId(snicHeader.GetFlowId());
    schedHeader.SetSourceIp(ipv4Header.GetSource());
    schedHeader.SetDestinationIp(ipv4Header.GetDestination());
    schedHeader.SetSourcePort(snicHeader.GetSourcePort());
    schedHeader.SetDestinationPort(snicHeader.GetDestinationPort());
    schedHeader.SetProtocol(snicHeader.GetProtocol());

    ipv4Header.SetDestination(m_schedulerAddress);
    ipv4Header.SetSource(m_ipAddress);

    snicHeader.SetPacketType(SnicHeader::ALLOCATION_REQUEST);

    request->AddHeader(schedHeader);
    request->AddHeader(snicHeader);
    request->AddHeader(ipv4Header);
    NS_LOG_DEBUG("flowid in snicheader: " << snicHeader.GetFlowId());
    NS_LOG_DEBUG("flowid in schedheader: " << schedHeader.GetFlowId());
    // create new flow to track packets
    FlowId flowId(schedHeader);
    // make the flow known to the packet cache
    PacketBuffer::Entry* entry = m_packetBuffer.Add(flowId);
    entry->SetIncomingPort(incomingPort);
    entry->SetProtocol(protocol);
    entry->SetSrc(src);
    entry->SetDst(dst);

    m_packetBuffer.SetWaitReplyTimeout(Seconds(5));
    entry->MarkWaitReply();
    entry->EnqueuePending(packet);
    //  newentry = flowid;
    //  m_packetBuffer.EnqueuePending(flowid, packet);
    //  m_packetBuffer.EnqueuePending(flowid, packet);
    m_rxCallback(this, request, protocol, m_address);
}

void
SnicNetDevice::AllocationRelease(Ptr<NetDevice> incomingPort,
                                 Ptr<const Packet> packet,
                                 uint16_t protocol,
                                 const Address& src,
                                 const Address& dst)
{
    NS_LOG_FUNCTION(this);

    Ptr<Packet> copy = packet->Copy();

    Ptr<Packet> request = Create<Packet>();
    Ipv4Header ipv4Header;
    SnicHeader snicHeader;
    copy->RemoveHeader(ipv4Header);
    copy->RemoveHeader(snicHeader);
    // copy->RemoveHeader(schedHeader);
    SnicSchedulerHeader schedHeader;
    // NS_LOG_DEBUG("flowid in schedheader: " << schedHeader.GetFlowId());
    // NS_LOG_DEBUG("flowid in schedheader: " << schedHeader.GetDestinationIp());
    NS_ASSERT_MSG(snicHeader.IsLastInFlow(), "is not last packet flow!");

    // schedHeader.SetBandwidthDemand(snicHeader.GetTput());
    //  schedHeader.SetBandwidthDemand(20.55);
    // schedHeader.SetResourceDemand(5);
    // schedHeader.AddNT(5);
    schedHeader.SetPacketType(SnicSchedulerHeader::ALLOCATION_RELEASE);

    schedHeader.SetFlowId(snicHeader.GetFlowId());
    schedHeader.SetSourceIp(ipv4Header.GetSource());
    schedHeader.SetDestinationIp(ipv4Header.GetDestination());
    schedHeader.SetSourcePort(snicHeader.GetSourcePort());
    schedHeader.SetDestinationPort(snicHeader.GetDestinationPort());
    schedHeader.SetProtocol(snicHeader.GetProtocol());

    ipv4Header.SetDestination(m_schedulerAddress);
    ipv4Header.SetSource(m_ipAddress);

    snicHeader.SetPacketType(SnicHeader::ALLOCATION_RELEASE);

    request->AddHeader(schedHeader);
    request->AddHeader(snicHeader);
    request->AddHeader(ipv4Header);
    NS_LOG_DEBUG("flowid in snicheader: " << snicHeader.GetFlowId());
    NS_LOG_DEBUG("flowid in schedheader: " << schedHeader.GetFlowId());
    // create new flow to track packets
    FlowId flowId(schedHeader);

    // NOTE maybe wait for response?
    m_packetBuffer.Delete(flowId);

    // NS_ASSERT_MSG(false, "sending release req");

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
SnicNetDevice::AddAddress(Mac48Address addr)
{
    NS_LOG_FUNCTION(this << addr);
    m_addresses[addr] = Mac48Address();
}

bool
SnicNetDevice::IsOurAddress(Mac48Address addr) const
{
    NS_LOG_FUNCTION(this << addr);
    return m_addresses.count(addr) > 0;
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
    // snicHeader.ClearRtes();
    NS_LOG_DEBUG("\tseen nic?: " << snicHeader.HasSeenNic());
    NS_LOG_DEBUG("\tis Scheduler?: " << IsScheduler());
    NS_LOG_DEBUG("\tpackettype?: " << snicHeader.GetPacketType());
    // packet->AddHeader(

    switch (snicHeader.GetPacketType())
    {
    case SnicHeader::L4_PACKET: {
        m_numL4Packets++;
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
                // ForwardUnicast(incomingPort, packet, protocol, src48, dst48);
                Forward(incomingPort, packet, protocol, src48, dst48);
                // if (snicHeader.IsLastInFlow())
                //{
                // AllocationRelease(incomingPort, packet, protocol, src, dst);
                //}

                return;
            }
            // this means the packet has been through one of the directly attached
            // sNICs
            snicHeader.SetHasSeenNic();
            if (IsScheduler())
            {
                NS_LOG_DEBUG("sched without alloc request NOW============");
                // ForwardBroadcast(
                // scheduler
            }
            else
            {
                // if we are forcing a new flow, then
                // if (snicHeader.IsNewFlow())
                //{
                //// snicHeader.SetFlowId(m_currentFlowId++);
                //// snicHeader.SetNewFlow(true);
                //// flow id
                //}
                // TODO check if there exist an packet buffer entry already for
                // this flow
                FlowId flowId(ipv4Header, snicHeader);
                NS_LOG_DEBUG("req sched ============" << snicHeader.GetFlowId());
                NS_LOG_DEBUG("req sched ============" << flowId.GetId());
                PacketBuffer::Entry* entry = m_packetBuffer.Lookup(flowId);
                // we have an entry here already, then we can set the sseensnic
                // flag and forward the packet
                //
                // Otherwise we need to send a allocation request to the
                // scheduler
                if (entry)
                {
                    if (entry->IsWaitReply())
                    {
                        packet->AddHeader(snicHeader);
                        packet->AddHeader(ipv4Header);
                        entry->EnqueuePending(packet);
                        NS_LOG_DEBUG("flow waitreply, enqueue");
                        // FIXME handle last packet in flow while waiting for
                        // scheduler response
                    }
                    else
                    {
                        snicHeader.SetRteList(entry->GetRoute());
                        packet->AddHeader(snicHeader);
                        packet->AddHeader(ipv4Header);
                        NS_LOG_DEBUG("flow not waitreply forward");
                        Forward(incomingPort, packet, protocol, src48, dst48);
                        if (snicHeader.IsLastInFlow())
                        {
                            NS_LOG_DEBUG("in later if");
                            AllocationRelease(incomingPort, packet, protocol, src, dst);
                        }
                    }
                }
                else
                {
                    NS_LOG_DEBUG("flow not found, requesting");
                    packet->AddHeader(snicHeader);
                    packet->AddHeader(ipv4Header);
                    AllocationRequest(incomingPort, packet, protocol, src, dst);
                    // FIXME also need to handle a single packet flow case.
                }
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
        m_numSchedReqs++;
        m_schedTrace(this, packet);
        SnicSchedulerHeader schedHeader;
        packet->RemoveHeader(schedHeader);

        // if we aren't the scheduler then something went wrong
        NS_ASSERT_MSG(IsScheduler(), "packet is addressed to us but we are not scheduler");
        // if (!IsScheduler())
        //{
        // NS_FATAL_ERROR("");
        //}
        NS_LOG_DEBUG("running sched");
        if (m_scheduler.Schedule(snicHeader, schedHeader) == false)
        {
            NS_FATAL_ERROR("out of resource");
        }
        NS_LOG_DEBUG("done running sched");
        NS_LOG_DEBUG(snicHeader);
        //  reply
        Ptr<Packet> response = Create<Packet>();
        SnicSchedulerHeader responseSchedHeader(ipv4Header, snicHeader);
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
        // NS_ASSERT_MSG(false, "debugging scheduler");
        NS_LOG_DEBUG("old src: " << ipv4Header.GetSource());
        NS_LOG_DEBUG("old dest: " << ipv4Header.GetDestination());
        NS_LOG_DEBUG(responseSnicHeader);

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
        HandleAllocationResponse(ipv4Header,
                                 snicHeader,
                                 incomingPort,
                                 packet,
                                 protocol,
                                 src48,
                                 dst48);
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
        HandleAllocationRelease(ipv4Header,
                                snicHeader,
                                incomingPort,
                                packet,
                                protocol,
                                src48,
                                dst48);
        // NS_ASSERT_MSG(false, "done release");
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
    NS_LOG_DEBUG("uid " << packet->GetUid());
    NS_LOG_DEBUG("snic id: " << GetSnicId());
    // NS_LOG_DEBUG("id is " << m_node->GetId());

    Mac48Address src48 = Mac48Address::ConvertFrom(src);
    Mac48Address dst48 = Mac48Address::ConvertFrom(dst);
    // NS_LOG_DEBUG("mac src is " << src48);
    // NS_LOG_DEBUG("mac dest is " << dst48);
    // NS_LOG_DEBUG("m_address is " << m_address);
    // NS_LOG_DEBUG("packetType is " << packetType);
    SnicHeader snicHeader;
    Ptr<Packet> copypkt = packet->Copy();
    // NS_LOG_DEBUG("test2");
    // copypkt->RemoveHeader(snicHeader);
    // packet->PeekHeader(snicHeader);
    // NS_LOG_DEBUG("test");
    std::ostringstream coll;

    packet->Print(coll);
    // snicHeader.Print(coll);

    NS_LOG_DEBUG("header is " << coll.str());
    // NS_LOG_DEBUG("header nt is " << snicHeader.GetNT());

    if (!m_promiscRxCallback.IsNull())
    {
        m_promiscRxCallback(this, packet, protocol, src, dst, packetType);
    }
    // bool isOurAddress = IsOurAddress(dst48);
    bool isOurAddress = dst48 == m_address;

    switch (packetType)
    {
    case PACKET_HOST:
        if (isOurAddress)
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
        if (m_broadcastedPackets.count(packet->GetUid()) > 0)
        {
            NS_LOG_DEBUG("packetType PACKET_MULTICAST already seen == dropping");
            break;
        }
        // if (protocol != ArpL3Protocol::PROT_NUMBER)
        m_rxCallback(this, packet, protocol, src);
        // warp snic header
        m_broadcastedPackets[packet->GetUid()] = 1;
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
            if (isOurAddress)
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
    // snicHeader.ClearRtes();

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
        PipelinedSendFrom(outPort, packet->Copy(), src, dst, protocol);
        // outPort->SendFrom(packet->Copy(), src, dst, protocol);
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
                // port->SendFrom(packet->Copy(), src, dst, protocol);
                PipelinedSendFrom(port, packet->Copy(), src, dst, protocol);
            }
        }
    }
}

void
SnicNetDevice::Forward(Ptr<NetDevice> incomingPort,
                       Ptr<Packet> packet,
                       uint16_t protocol,
                       Mac48Address src,
                       Mac48Address dst)
{
    NS_LOG_FUNCTION(this);

    Ipv4Header ipv4Header;
    SnicHeader snicHeader;
    packet->RemoveHeader(ipv4Header);
    packet->RemoveHeader(snicHeader);
    std::list<SnicRte> rteList = snicHeader.GetRteList();

    bool hasUnprocessedRte = false;

    Ptr<NetDevice> nextDevice = nullptr;

    NS_LOG_DEBUG("rte size=" << rteList.size());
    if (snicHeader.GetUseRouting() && rteList.size() > 0)
    {
        // find next rte entry.
        for (auto it = rteList.begin(); it != rteList.end(); ++it)
        {
            SnicRte& rte = *it;
            // NS_LOG_DEBUG("proc= " << rte.GetProcessed());
            // NS_LOG_DEBUG("l= " << rte.GetLDevice());
            // NS_LOG_DEBUG("r= " << rte.GetRDevice());
            // NS_LOG_DEBUG("this= " << this);
            // NS_LOG_DEBUG("======");
            //  if ((*it).GetLDevice() == this)
            //{
            //  NS_LOG_DEBUG("found ourself in rte list");
            // }

            // skip processed rte
            if (rte.GetProcessed())
            {
                continue;
            }
            hasUnprocessedRte = true;
            NS_LOG_DEBUG("Not processed rte");
            for (std::vector<Ptr<NetDevice>>::iterator iter = m_ports.begin();
                 iter != m_ports.end();
                 iter++)
            {
                Ptr<NetDevice> port = *iter;
                if (port == (*it).GetRDevice())
                {
                    NS_LOG_DEBUG("found in imp list");
                    rte.SetProcessed(true);
                    nextDevice = port;

                    snicHeader.SetRteList(rteList);
                    break;
                }
            }
            if (nextDevice)
            {
                break;
            }
            else
            {
                NS_ASSERT_MSG(false, "didn't find a device for unprocessed rte");
            }
        }

        packet->AddHeader(snicHeader);
        packet->AddHeader(ipv4Header);
        if (hasUnprocessedRte == false)
        {
            NS_LOG_DEBUG("last hop");
            ForwardUnicast(incomingPort, packet, protocol, src, dst);
            return;
        }
        else if (nextDevice)
        {
            PipelinedSendFrom(nextDevice, packet->Copy(), src, dst, protocol);
            return;
        }
        else
        {
            NS_ASSERT_MSG(false, "didn't find rte");
        }
        NS_ASSERT_MSG(false, "shouldn't be reached");
    }
    else
    {
        NS_ASSERT("no rteList");
        packet->AddHeader(snicHeader);
        packet->AddHeader(ipv4Header);
        ForwardUnicast(incomingPort, packet, protocol, src, dst);
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
            // port->SendFrom(packet->Copy(), src, dst, protocol);
            PipelinedSendFrom(port, packet->Copy(), src, dst, protocol);
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
SnicNetDevice::HandleAllocationRequest(Ipv4Header& ipv4Header,
                                       SnicHeader& snicHeader,
                                       Ptr<NetDevice> incomingPort,
                                       Ptr<Packet> packet,
                                       uint16_t protocol,
                                       Mac48Address src,
                                       Mac48Address dst)
{
}

void
SnicNetDevice::HandleAllocationResponse(Ipv4Header& ipv4Header,
                                        SnicHeader& snicHeader,
                                        Ptr<NetDevice> incomingPort,
                                        Ptr<Packet> packet,
                                        uint16_t protocol,
                                        Mac48Address src,
                                        Mac48Address dst)
{
    NS_LOG_FUNCTION(this);

    SnicSchedulerHeader schedHeader;
    packet->PeekHeader(schedHeader);
    NS_LOG_DEBUG("got response");
    // get flowid from response packet
    FlowId flowId(schedHeader);

    // save route to use in other packets of the flow.
    //
    // find entry in packet buffer
    PacketBuffer::Entry* entry = m_packetBuffer.Lookup(flowId);
    if (entry)
    {
        NS_LOG_DEBUG("found entry");
        entry->SetRoute(snicHeader.GetRteList());
    }

    Ptr<Packet> pending = entry->DequeuePending();
    while (pending)
    {
        // send
        Mac48Address src48 = Mac48Address::ConvertFrom(entry->GetSrc());
        Mac48Address dst48 = Mac48Address::ConvertFrom(entry->GetDst());
        NS_LOG_DEBUG("dequeue uid " << pending->GetUid());
        // packet->AddHeader(snicHeader);
        // packet->AddHeader(ipv4Header);
        Ipv4Header pendingIpv4Header;
        SnicHeader pendingSnicHeader;
        pending->RemoveHeader(pendingIpv4Header);
        pending->RemoveHeader(pendingSnicHeader);
        pendingSnicHeader.SetRteList(snicHeader.GetRteList());

        pending->AddHeader(pendingSnicHeader);
        pending->AddHeader(pendingIpv4Header);

        // NS_LOG_DEBUG(pendingSnicHeader);

        Forward(entry->GetIncomingPort(), pending, entry->GetProtocol(), src48, dst48);
        if (pendingSnicHeader.IsLastInFlow())
        {
            AllocationRelease(incomingPort, pending, protocol, src, dst);
        }
        pending = entry->DequeuePending();
    }
    entry->MarkActive();
}

void
SnicNetDevice::HandleAllocationRelease(Ipv4Header& ipv4Header,
                                       SnicHeader& snicHeader,
                                       Ptr<NetDevice> incomingPort,
                                       Ptr<Packet> packet,
                                       uint16_t protocol,
                                       Mac48Address src,
                                       Mac48Address dst)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(IsScheduler(), "we aren't the scheduler!");

    SnicSchedulerHeader schedHeader;
    packet->RemoveHeader(schedHeader);
    m_scheduler.Release(snicHeader, schedHeader);
}

void
SnicNetDevice::PipelinedSendFrom(Ptr<NetDevice> port,
                                 Ptr<Packet> packet,
                                 const Address& src,
                                 const Address& dst,
                                 uint16_t protocol)
{
    NS_LOG_FUNCTION(this);
    Ipv4Header ipv4Header;
    SnicHeader snicHeader;
    // int size = packet->RemoveHeader(ipv4Header);
    // NS_LOG_DEBUG("size: " << size);
    //  packet->PeekHeader(ipv4Header);
    // packet->AddHeader(ipv4Header);
    //    Time delay = snicHeader.GetDelay();

    // Time now = Simulator::Now();

    // m_pipelineLength = 16;
    // m_numInPipeline++;
    //  NS_LOG_DEBUG("delay=" << delay << " inPipe=" << m_numInPipeline);

    // if (m_numInPipeline >= 16)
    //{
    //  NS_
    //}
    // port->SendFrom(packet, src, dst, protocol);

    // m_pipeline.push(
    Time delay = NanoSeconds(0);

    NS_LOG_DEBUG("scheduling packet send in: " << delay);
    Simulator::Schedule(delay, &NetDevice::SendFrom, port, packet, src, dst, protocol);
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

    Mac48Address dst = Mac48Address::ConvertFrom(dest);

    // try to use the learned state if data is unicast
    if (!dst.IsGroup())
    {
        Ptr<NetDevice> outPort = GetLearnedState(dst);
        if (outPort)
        {
            // outPort->SendFrom(packet, source, dest, protocolNumber);
            PipelinedSendFrom(outPort, packet, source, dest, protocolNumber);
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
        // port->SendFrom(pktCopy, source, dest, protocolNumber);
        PipelinedSendFrom(port, pktCopy, source, dest, protocolNumber);
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
