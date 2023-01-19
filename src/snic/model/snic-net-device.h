/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_NET_DEVICE_H
#define SNIC_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/traced-callback.h"

#include <map>
#include <stdint.h>
#include <string>
#include <tuple>

namespace ns3
{
  class Node;

  class SnicNetDevice : public NetDevice
  {
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
    /**
     * \brief Gets the number of snic connected 'ports', i.e., the NetDevices currently connected to snic.
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

    // inherited from NetDevice base class
    void SetIfIndex(const uint32_t index) override;
    uint32_t GetIfIndex() const override;
    Ptr<Channel> GetChannel() const override;
    void SetAddress(Address address) override;
    Address GetAddress() const override;
    bool SetMtu(const uint16_t mtu) override;

    /**
     * \brief Returns the link-layer MTU for this interface.
     * If the link-layer MTU is smaller than IPv6's minimum MTU (\RFC{4944}),
     * 1280 will be returned.
     *
     * \return The link-level MTU in bytes for this interface.
     */
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

  protected:
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
                           Ptr<const Packet> packet,
                           uint16_t protocol,
                           const Address& src,
                           const Address& dst,
                           PacketType packetType);
  private:
    uint16_t m_num_hosts_connected;
    uint16_t m_num_ports;
    /**
     * \brief Dispose of the object
     */
    void DoDispose() override;

    /**
     * \returns the address of the remote device connected to this device
     * through the point to point channel.
     */
    Address GetRemote() const;

    /**
     * Adds the necessary headers and trailers to a packet of data in order to
     * respect the protocol implemented by the agent.
     * \param p packet
     * \param protocolNumber protocol number
     */
    void AddHeader(Ptr<Packet> p, uint16_t protocolNumber);

    /**
     * Removes, from a packet of data, all headers and trailers that
     * relate to the protocol implemented by the agent
     * \param p Packet whose headers need to be processed
     * \param param An integer parameter that can be set by the function
     * \return Returns true if the packet should be forwarded up the
     * protocol stack.
     */
    bool ProcessHeader(Ptr<Packet> p, uint16_t& param);

    /**
     * Start Sending a Packet Down the Wire.
     *
     * The TransmitStart method is the method that is used internally in the
     * PointToPointNetDevice to begin the process of sending a packet out on
     * the channel.  The corresponding method is called on the channel to let
     * it know that the physical device this class represents has virtually
     * started sending signals.  An event is scheduled for the time at which
     * the bits have been completely transmitted.
     *
     * \see PointToPointChannel::TransmitStart ()
     * \see TransmitComplete()
     * \param p a reference to the packet to send
     * \returns true if success, false on failure
     */
    bool TransmitStart(Ptr<Packet> p);

    /**
     * Stop Sending a Packet Down the Wire and Begin the Interframe Gap.
     *
     * The TransmitComplete method is used internally to finish the process
     * of sending a packet out on the channel.
     */
    void TransmitComplete();

    /**
     * \brief Make the link up and running
     *
     * It calls also the linkChange callback.
     */
    void NotifyLinkUp();

    /**
     * Enumeration of the states of the transmit machine of the net device.
     */
    enum TxMachineState
    {
        READY, /**< The transmitter is ready to begin transmission of a packet */
        BUSY   /**< The transmitter is busy transmitting a packet */
    };

    /**
     * The state of the Net Device transmit state machine.
     */
    TxMachineState m_txMachineState;

    /**
     * The data rate that the Net Device uses to simulate packet transmission
     * timing.
     */
    DataRate m_bps;

    /**
     * The interframe gap that the Net Device uses to throttle packet
     * transmission
     */
    Time m_tInterframeGap;

    /**
     * The PointToPointChannel to which this PointToPointNetDevice has been
     * attached.
     */
    Ptr<PointToPointChannel> m_channel;

    /**
     * The Queue which this PointToPointNetDevice uses as a packet source.
     * Management of this Queue has been delegated to the PointToPointNetDevice
     * and it has the responsibility for deletion.
     * \see class DropTailQueue
     */
    Ptr<Queue<Packet>> m_queue;

    /**
     * Error model for receive packet events
     */
    Ptr<ErrorModel> m_receiveErrorModel;

    /**
     * The trace source fired when packets come into the "top" of the device
     * at the L3/L2 transition, before being queued for transmission.
     */
    TracedCallback<Ptr<const Packet>> m_macTxTrace;

    /**
     * The trace source fired when packets coming into the "top" of the device
     * at the L3/L2 transition are dropped before being queued for transmission.
     */
    TracedCallback<Ptr<const Packet>> m_macTxDropTrace;

    /**
     * The trace source fired for packets successfully received by the device
     * immediately before being forwarded up to higher layers (at the L2/L3
     * transition).  This is a promiscuous trace (which doesn't mean a lot here
     * in the point-to-point device).
     */
    TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;

    /**
     * The trace source fired for packets successfully received by the device
     * immediately before being forwarded up to higher layers (at the L2/L3
     * transition).  This is a non-promiscuous trace (which doesn't mean a lot
     * here in the point-to-point device).
     */
    TracedCallback<Ptr<const Packet>> m_macRxTrace;

    /**
     * The trace source fired for packets successfully received by the device
     * but are dropped before being forwarded up to higher layers (at the L2/L3
     * transition).
     */
    TracedCallback<Ptr<const Packet>> m_macRxDropTrace;

    /**
     * The trace source fired when a packet begins the transmission process on
     * the medium.
     */
    TracedCallback<Ptr<const Packet>> m_phyTxBeginTrace;

    /**
     * The trace source fired when a packet ends the transmission process on
     * the medium.
     */
    TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;

    /**
     * The trace source fired when the phy layer drops a packet before it tries
     * to transmit it.
     */
    TracedCallback<Ptr<const Packet>> m_phyTxDropTrace;

    /**
     * The trace source fired when a packet begins the reception process from
     * the medium -- when the simulated first bit(s) arrive.
     */
    TracedCallback<Ptr<const Packet>> m_phyRxBeginTrace;

    /**
     * The trace source fired when a packet ends the reception process from
     * the medium.
     */
    TracedCallback<Ptr<const Packet>> m_phyRxEndTrace;

    /**
     * The trace source fired when the phy layer drops a packet it has received.
     * This happens if the receiver is not enabled or the error model is active
     * and indicates that the packet is corrupt.
     */
    TracedCallback<Ptr<const Packet>> m_phyRxDropTrace;

    /**
     * A trace source that emulates a non-promiscuous protocol sniffer connected
     * to the device.  Unlike your average everyday sniffer, this trace source
     * will not fire on PACKET_OTHERHOST events.
     *
     * On the transmit size, this trace hook will fire after a packet is dequeued
     * from the device queue for transmission.  In Linux, for example, this would
     * correspond to the point just before a device \c hard_start_xmit where
     * \c dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET
     * ETH_P_ALL handlers.
     *
     * On the receive side, this trace hook will fire when a packet is received,
     * just before the receive callback is executed.  In Linux, for example,
     * this would correspond to the point at which the packet is dispatched to
     * packet sniffers in \c netif_receive_skb.
     */
    TracedCallback<Ptr<const Packet>> m_snifferTrace;

    /**
     * A trace source that emulates a promiscuous mode protocol sniffer connected
     * to the device.  This trace source fire on packets destined for any host
     * just like your average everyday packet sniffer.
     *
     * On the transmit size, this trace hook will fire after a packet is dequeued
     * from the device queue for transmission.  In Linux, for example, this would
     * correspond to the point just before a device \c hard_start_xmit where
     * \c dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET
     * ETH_P_ALL handlers.
     *
     * On the receive side, this trace hook will fire when a packet is received,
     * just before the receive callback is executed.  In Linux, for example,
     * this would correspond to the point at which the packet is dispatched to
     * packet sniffers in \c netif_receive_skb.
     */
    TracedCallback<Ptr<const Packet>> m_promiscSnifferTrace;

    Ptr<Node> m_node;                                    //!< Node owning this NetDevice
    Mac48Address m_address;                              //!< Mac48Address of this NetDevice
    NetDevice::ReceiveCallback m_rxCallback;             //!< Receive callback
    NetDevice::PromiscReceiveCallback m_promiscCallback; //!< Receive callback
                                                         //   (promisc data)
    uint32_t m_ifIndex;                                  //!< Index of the interface
    bool m_linkUp;                                       //!< Identify if the link is up or not
    TracedCallback<> m_linkChangeCallbacks;              //!< Callback for the link change event

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

    Ptr<Packet> m_currentPkt; //!< Current packet processed

    /**
     * \brief PPP to Ethernet protocol number mapping
     * \param protocol A PPP protocol number
     * \return The corresponding Ethernet protocol number
     */
    static uint16_t PppToEther(uint16_t protocol);

    /**
     * \brief Ethernet to PPP protocol number mapping
     * \param protocol An Ethernet protocol number
     * \return The corresponding PPP protocol number
     */
    static uint16_t EtherToPpp(uint16_t protocol);
  };
}
#endif // SNIC_NET_DEVICE_H
