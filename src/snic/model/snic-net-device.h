/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_NET_DEVICE_H
#define SNIC_NET_DEVICE_H

//#include "snic-channel.h"

#include "ns3/arp-header.h"
#include "ns3/arp-l3-protocol.h"
#include "ns3/bridge-channel.h"
#include "ns3/enum.h"
#include "ns3/ethernet-header.h"
#include "ns3/integer.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/snic-module.h"
#include "ns3/string.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/uinteger.h"

#include <map>
#include <stdint.h>
#include <string>
#include <tuple>

namespace ns3
{
class Node;

class SnicNetDevice : public NetDevice
{
  public:
    /**
     * \brief Get the type ID.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Constructor for the SixLowPanNetDevice.
     */
    SnicNetDevice();
    ~SnicNetDevice() override;

    // Delete copy constructor and assignment operator to avoid misuse
    SnicNetDevice(const SnicNetDevice&) = delete;
    SnicNetDevice& operator=(const SnicNetDevice&) = delete;

    /**
     * \brief Add a 'port' to a snic device
     * \param NetDevice to add
     *
     * This method adds a new snic port to a BridgeNetDevice, so that
     * the new snic connected port NetDevice becomes part of the bridge and L2
     * frames start being forwarded to/from this NetDevice. XXX
     *
     * \attention The netdevice that is being added as bridge port must
     * _not_ have an IP address.  In order to add IP connectivity to a
     * bridging node you must enable IP on the BridgeNetDevice itself,
     * never on its port netdevices. XXX
     */
    void AddSnicPort(Ptr<NetDevice> snicPort);
    void AddSnicPort(Ptr<NetDevice> snicPort, bool isPeerSnic);
    // void AddPeerSnic(Ptr<SnicNetDevice> peerSnic, Ptr<SnicChannel> ch);

    /**
     * \brief Gets the number of snic connected 'ports', i.e., the NetDevices currently connected
     * to snic.
     *
     * \return the number of snic connected ports.
     */
    uint32_t GetNSnicPorts() const;

    /**
     * \brief Gets the n-th snic connected port.
     * \param n the port index
     * \return the n-th snic conected NetDevice
     */
    Ptr<NetDevice> GetSnicPort(uint32_t n) const;

    void AddNT(Ptr<NetworkTask> nt, uint32_t id);
    void RemoveNT(uint32_t id);
    Ptr<NetworkTask> GetNT(uint32_t id);
    uint32_t GetNumNT();

    void RequestAllocation(Ptr<NetDevice> incomingPort,
                           Ptr<Packet> packet,
                           uint16_t protocol,
                           const Address& src,
                           const Address& dst);

    void SetSchedulerAddress(Ipv4Address schedulerAddress);
    Ipv4Address GetSchedulerAddress() const;

    void SetIsScheduler(bool isScheduler);
    bool IsScheduler() const;
    void SetIpAddress(Ipv4Address address);
    Ipv4Address GetIpAddress() const;

    // int GetSnicPortIndex(Ptr<SnicPort>

    // inherited from NetDevice base class
    void SetIfIndex(const uint32_t index) override;
    uint32_t GetIfIndex() const override;
    Ptr<Channel> GetChannel() const override;
    void SetAddress(Address address) override;
    Address GetAddress() const override;
    bool SetMtu(const uint16_t mtu) override;
    uint16_t GetMtu() const override;
    bool IsLinkUp() const override;
    void AddLinkChangeCallback(Callback<void> callback) override;
    bool IsBroadcast() const override;
    Address GetBroadcast() const override;
    bool IsMulticast() const override;
    Address GetMulticast(Ipv4Address multicastGroup) const override;
    bool IsPointToPoint() const override;
    bool IsBridge() const override;
    bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;
    bool SendFrom(Ptr<Packet> packet,
                  const Address& source,
                  const Address& dest,
                  uint16_t protocolNumber) override;
    Ptr<Node> GetNode() const override;
    void SetNode(Ptr<Node> node) override;
    bool NeedsArp() const override;
    void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
    void SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb) override;
    bool SupportsSendFrom() const override;
    Address GetMulticast(Ipv6Address addr) const override;

    /**
     * Receive a packet from a connected CsmaChannel.
     *
     * The CsmaNetDevice receives packets from its connected channel
     * and forwards them up the protocol stack.  This is the public method
     * used by the channel to indicate that the last bit of a packet has
     * arrived at the device.
     *
     * \see CsmaChannel
     * \param p a reference to the received packet
     * \param sender the CsmaNetDevice that transmitted the packet in the first place
     */
    void Receive(Ptr<Packet> p, Ptr<SnicNetDevice> sender);

  protected:
    void AddAddress(Mac48Address addr);
    bool IsOurAddress(Mac48Address addr) const;
    void DoDispose() override;

    /**
     * Called when a packet is received on one of the switch's ports.
     *
     * \param netdev The port the packet was received on.
     * \param packet The Packet itself.
     * \param protocol The protocol defining the Packet.
     * \param src The source address of the Packet.
     * \param dst The destination address of the Packet.
     * \param packetType Type of the packet.
     */
    void ReceiveFromDevice(Ptr<NetDevice> netdev,
                           Ptr<Packet> packet,
                           uint16_t protocol,
                           const Address& src,
                           const Address& dst,
                           PacketType packetType);

    void HandleIpv4Packet(Ptr<NetDevice> incomingPort,
                          Ptr<Packet> packet,
                          uint16_t protocol,
                          const Address& src,
                          const Address& dst);

    /**
     * \brief Process a packet
     * \param incomingPort the packet incoming port
     * \param packet the packet
     * \param protocol the packet protocol (e.g., Ethertype)
     * \param src the packet source
     * \param dst the packet destination
     */
    Ptr<Packet> ProcessPacket(Ptr<NetDevice> incomingPort,
                              Ptr<const Packet> packet,
                              uint16_t protocol,
                              const Address& src,
                              const Address& dst);

    /**
     * \brief Forwards a unicast packet
     * \param incomingPort the packet incoming port
     * \param packet the packet
     * \param protocol the packet protocol (e.g., Ethertype)
     * \param src the packet source
     * \param dst the packet destination
     */
    void ForwardUnicast(Ptr<NetDevice> incomingPort,
                        Ptr<const Packet> packet,
                        uint16_t protocol,
                        Mac48Address src,
                        Mac48Address dst);

    /**
     * \brief Forwards a broadcast or a multicast packet
     * \param incomingPort the packet incoming port
     * \param packet the packet
     * \param protocol the packet protocol (e.g., Ethertype)
     * \param src the packet source
     * \param dst the packet destination
     */
    void ForwardBroadcast(Ptr<NetDevice> incomingPort,
                          Ptr<const Packet> packet,
                          uint16_t protocol,
                          Mac48Address src,
                          Mac48Address dst);

    /**
     * \brief Learns the port a MAC address is sending from
     * \param source source address
     * \param port the port the source is sending from
     */
    void Learn(Mac48Address source, Ptr<NetDevice> port);

    /**
     * \brief Gets the port associated to a source address
     * \param source the source address
     * \returns the port the source is associated to, or NULL if no association is known.
     */
    Ptr<NetDevice> GetLearnedState(Mac48Address source);

  private:
    static const uint16_t IPV4_PROT_NUMBER = 0x0800; //!< Protocol number (0x0800)
    uint16_t m_num_hosts_connected;
    uint16_t m_num_ports;

    std::map<uint32_t, Ptr<NetworkTask>> m_nts;

    /**
     * Add a flow.
     *
     * Possible error numbers: ENOMEM, ENOBUFS, ESRCH
     *
     * \param ofm The flow data to add.
     * \return 0 if everything's ok, otherwise an error number.
     */
    // int AddFlow(const ofp_flow_mod* ofm);

    /**
     * Modify a flow.
     *
     * \param ofm The flow data to modify.
     * \return 0 if everything's ok, otherwise an error number.
     */
    // int ModFlow(const ofp_flow_mod* ofm);

    /**
     * Send packets out all the ports except the originating one
     *
     * \param packet_uid Packet UID; used to fetch the packet and its metadata.
     * \param in_port The index of the port the Packet was initially received on. This port doesn't
     * forward when flooding.
     * \param flood If true, don't send out on the ports with flooding disabled.
     * \return 0 if everything's ok, otherwise an error number.
     */
    int OutputAll(uint32_t packet_uid, int in_port, bool flood);

    /**
     * Sends a copy of the Packet over the provided output port
     *
     * \param packet_uid Packet UID; used to fetch the packet and its metadata.
     * \param out_port Output port.
     */
    void OutputPacket(uint32_t packet_uid, int out_port);

    /**
     * Seeks to send out a Packet over the provided output port. This is called generically
     * when we may or may not know the specific port we're outputting on. There are many
     * pre-set types of port options besides a Port that's hooked to our OpenFlowSwitchNetDevice.
     * For example, it could be outputting as a flood, or seeking to output to the controller.
     *
     * \param packet_uid Packet UID; used to fetch the packet and its metadata.
     * \param in_port The index of the port the Packet was initially received on.
     * \param out_port The port we want to output on.
     * \param ignore_no_fwd If true, Ports that are set to not forward are forced to forward.
     */
    void OutputPort(uint32_t packet_uid, int in_port, int out_port, bool ignore_no_fwd);

    /**
     * Sends a copy of the Packet to the controller. If the packet can be saved
     * in an OpenFlow buffer, then only the first 'max_len' bytes of the packet
     * are sent; otherwise, all of the packet is sent.
     *
     * \param packet_uid Packet UID; used to fetch the packet and its metadata.
     * \param in_port The index of the port the Packet was initially received on.
     * \param max_len The maximum number of bytes that the caller wants to be sent; a value of 0
     * indicates the entire packet should be sent.
     * \param reason Why the packet is being sent.
     */
    void OutputControl(uint32_t packet_uid, int in_port, size_t max_len, int reason);

    /**
     * If an error message happened during the controller's request, send it to the controller.
     *
     * \param type The type of error.
     * \param code The error code.
     * \param data The faulty data that lead to the error.
     * \param len The length of the faulty data.
     */
    void SendErrorMsg(uint16_t type, uint16_t code, const void* data, size_t len);

    /**
     * Send a reply about this OpenFlow switch's features to the controller.
     *
     * List of capabilities and actions to support are found in the specification
     * <www.openflowswitch.org/documents/openflow-spec-v0.8.9.pdf>.
     *
     * Supported capabilities and actions are defined in the openflow interface.
     * To recap, flow status, flow table status, port status, virtual port table
     * status can all be requested. It can also transmit over multiple physical
     * interfaces.
     *
     * It supports every action: outputting over a port, and all of the flow table
     * manipulation actions: setting the 802.1q VLAN ID, the 802.1q priority,
     * stripping the 802.1 header, setting the Ethernet source address and destination,
     * setting the IP source address and destination, setting the TCP/UDP source address
     * and destination, and setting the MPLS label and EXP bits.
     *
     * \attention Capabilities STP (Spanning Tree Protocol) and IP packet
     * reassembly are not currently supported.
     *
     */
    void SendFeaturesReply();

    /**
     * Send a reply to the controller that a specific flow has expired.
     *
     * \param flow The flow that expired.
     * \param reason The reason for sending this expiration notification.
     */
    // void SendFlowExpired(sw_flow* flow, enum ofp_flow_expired_reason reason);

    /**
     * Send a reply about a Port's status to the controller.
     *
     * \param p The port to get status from.
     * \param status The reason for sending this reply.
     */
    // void SendPortStatus(ofi::Port p, uint8_t status);

    /**
     * Send a reply about this OpenFlow switch's virtual port table features to the controller.
     */
    void SendVPortTableFeatures();

    /**
     * Send a message to the controller. This method is the key
     * to communicating with the controller, it does the actual
     * sending. The other Send methods call this one when they
     * are ready to send a message.
     *
     * \param buffer Buffer of the message to send out.
     * \return 0 if successful, otherwise an error number.
     */
    // int SendOpenflowBuffer(ofpbuf* buffer);

    /**
     * Run the packet through the flow table. Looks up in the flow table for a match.
     * If it doesn't match, it forwards the packet to the registered controller, if the flag is set.
     *
     * \param packet_uid Packet UID; used to fetch the packet and its metadata.
     * \param port The port this packet was received over.
     * \param send_to_controller If set, sends to the controller if the packet isn't matched.
     */
    void RunThroughFlowTable(uint32_t packet_uid, int port, bool send_to_controller = true);

    /**
     * Run the packet through the vport table. As with AddVPort,
     * this doesn't have an understood use yet.
     *
     * \param packet_uid Packet UID; used to fetch the packet and its metadata.
     * \param port The port this packet was received over.
     * \param vport The virtual port this packet identifies itself by.
     * \return 0 if everything's ok, otherwise an error number.
     */
    int RunThroughVPortTable(uint32_t packet_uid, int port, uint32_t vport);

    /**
     * Called by RunThroughFlowTable on a scheduled delay
     * to account for the flow table lookup overhead.
     *
     * \param key Matching key to look up in the flow table.
     * \param buffer Buffer of the packet received.
     * \param packet_uid Packet UID; used to fetch the packet and its metadata.
     * \param port The port the packet was received over.
     * \param send_to_controller
     */
    // void FlowTableLookup(sw_flow_key key,
    // ofpbuf* buffer,
    // uint32_t packet_uid,
    // int port,
    // bool send_to_controller);

    /**
     * Update the port status field of the switch port.
     * A non-zero return value indicates some field has changed.
     *
     * \param p A reference to a Port; used to change its config and flag fields.
     * \return true if the status of the Port is changed, false if unchanged (was already the right
     * status).
     */
    // int UpdatePortStatus(ofi::Port& p);

    /**
     * Fill out a description of the switch port.
     *
     * \param p The port to get the description from.
     * \param desc A pointer to the description message; used to fill the description message with
     * the data from the port.
     */
    // void FillPortDesc(ofi::Port p, ofp_phy_port* desc);

    /**
     * Generates an OpenFlow reply message based on the type.
     *
     * \param openflow_len Length of the reply to make.
     * \param type Type of reply message to make.
     * \param bufferp Message buffer; used to make the reply.
     * \return The OpenFlow reply message.
     */
    // void* MakeOpenflowReply(size_t openflow_len, uint8_t type, ofpbuf** bufferp);

    /**
     * \name Receive Methods
     *
     * Actions to do when a specific OpenFlow message/packet is received
     *
     * @{
     */
    /**
     * \param msg The OpenFlow message received.
     * \return 0 if everything's ok, otherwise an error number.
     */
    int ReceiveFeaturesRequest(const void* msg);
    int ReceiveGetConfigRequest(const void* msg);
    int ReceiveSetConfig(const void* msg);
    int ReceivePacketOut(const void* msg);
    int ReceiveFlow(const void* msg);
    int ReceivePortMod(const void* msg);
    int ReceiveStatsRequest(const void* msg);
    int ReceiveEchoRequest(const void* msg);
    int ReceiveEchoReply(const void* msg);
    int ReceiveVPortMod(const void* msg);
    int ReceiveVPortTableFeaturesRequest(const void* msg);
    /**@}*/

    /**
     * \ingroup bridge
     * Structure holding the status of an address
     */
    struct LearnedState
    {
        Ptr<NetDevice> associatedPort; //!< port associated with the address
        Time expirationTime;           //!< time it takes for learned MAC state to expire
    };

    std::map<Mac48Address, LearnedState> m_learnState;   //!< Container for known address statuses
    Ptr<Node> m_node;                                    //!< Node owning this NetDevice
    Mac48Address m_address;                              //!< Mac48Address of this NetDevice
    std::map<Mac48Address, Mac48Address> m_addresses;    //!< All Mac48Addresses of this NetDevice
    Ipv4Address m_ipAddress;                             //!< Ipv4Address of this NetDevice
    Time m_expirationTime; //!< time it takes for learned MAC state to expire
                           //
    Ptr<BridgeChannel> m_channel;                        //!< virtual bridged channel
    std::vector<Ptr<NetDevice>> m_ports;                 //!< bridged ports
    NetDevice::ReceiveCallback m_rxCallback;             //!< Receive callback
    NetDevice::PromiscReceiveCallback m_promiscRxCallback; //!< Receive callback
                           //   (promisc data)
    uint32_t m_ifIndex;    //!< Index of the interface
                           //

    static const uint16_t DEFAULT_MTU = 1500; //!< Default MTU

    /**
     * \brief The Maximum Transmission Unit
     *
     * This corresponds to the maximum
     * number of bytes that can be transmitted as seen from higher layers.
     * This corresponds to the 1500 byte MTU size often seen on IP over
     * Ethernet.
     */
    uint32_t m_mtu;
    bool m_enableLearning; //!< true if the bridge will learn the node status

    /// PacketData type
    // typedef std::map<uint32_t, ofi::SwitchPacketMetadata> PacketData_t;
    // PacketData_t m_packetData; ///< Packet data

    Ptr<Packet> m_currentPkt; //!< Current packet processed
    bool m_isScheduler;
    Ipv4Address m_schedulerAddress;
    SnicScheduler m_scheduler;
    std::vector<Address> m_connectedHosts;
    std::vector<Address> m_connectedSnics;
    std::map<uint32_t, uint32_t> m_broadcastedPackets;

    TracedValue<uint64_t> m_numSchedReqs;
    TracedValue<uint64_t> m_numL4Packets;

    PacketBuffer m_packetBuffer;
  };
}
#endif // SNIC_NET_DEVICE_H
