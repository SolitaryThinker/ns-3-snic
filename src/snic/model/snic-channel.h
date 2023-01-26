/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_CHANNEL_H
#define SNIC_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3
{

class Packet;

class SnicChannel : public Channel
{
  public:
    /**
     * \brief Get the TypeId
     *
     * \return The TypeId for this class
     */
    static TypeId GetTypeId();

    /**
     * \brief Create a SnicChannel
     *
     * By default, you get a channel that has an "infinitely" fast
     * transmission speed and zero delay.
     */
    SnicChannel();

    /**
     * \brief Attach a given netdevice to this channel
     * \param device pointer to the netdevice to attach to the channel
     */
    void Attach(Ptr<SnicNetDevice> device);

    /**
     * \brief Transmit a packet over this channel
     * \param p Packet to transmit
     * \param src Source SnicNetDevice
     * \param txTime Transmit time to apply
     * \returns true if successful (currently always true)
     */
    virtual bool TransmitStart(Ptr<const Packet> p, Ptr<SnicNetDevice> src, Time txTime);

    /**
     * \brief Get number of devices on this channel
     * \returns number of devices on this channel
     */
    std::size_t GetNDevices() const override;

    /**
     * \brief Get SnicNetDevice corresponding to index i on this channel
     * \param i Index number of the device requested
     * \returns Ptr to SnicNetDevice requested
     */
    Ptr<SnicNetDevice> GetSnicDevice(std::size_t i) const;

    /**
     * \brief Get NetDevice corresponding to index i on this channel
     * \param i Index number of the device requested
     * \returns Ptr to NetDevice requested
     */
    Ptr<NetDevice> GetDevice(std::size_t i) const override;

  protected:
    /**
     * \brief Get the delay associated with this channel
     * \returns Time delay
     */
    Time GetDelay() const;

    /**
     * \brief Check to make sure the link is initialized
     * \returns true if initialized, asserts otherwise
     */
    bool IsInitialized() const;

    /**
     * \brief Get the net-device source
     * \param i the link requested
     * \returns Ptr to SnicNetDevice source for the
     * specified link
     */
    Ptr<SnicNetDevice> GetSource(uint32_t i) const;

    /**
     * \brief Get the net-device destination
     * \param i the link requested
     * \returns Ptr to SnicNetDevice destination for
     * the specified link
     */
    Ptr<SnicNetDevice> GetDestination(uint32_t i) const;

    /**
     * TracedCallback signature for packet transmission animation events.
     *
     * \param [in] packet The packet being transmitted.
     * \param [in] txDevice the TransmitTing NetDevice.
     * \param [in] rxDevice the Receiving NetDevice.
     * \param [in] duration The amount of time to transmit the packet.
     * \param [in] lastBitTime Last bit receive time (relative to now)
     * \deprecated The non-const \c Ptr<NetDevice> argument is deprecated
     * and will be changed to \c Ptr<const NetDevice> in a future release.
     */
    typedef void (*TxRxAnimationCallback)(Ptr<const Packet> packet,
                                          Ptr<NetDevice> txDevice,
                                          Ptr<NetDevice> rxDevice,
                                          Time duration,
                                          Time lastBitTime);

  private:
    /** Each sNic link has exactly two net devices. */
    static const std::size_t N_DEVICES = 2;

    Time m_delay;           //!< Propagation delay
    std::size_t m_nDevices; //!< Devices of this channel

    /**
     * The trace source for the packet transmission animation events that the
     * device can fire.
     * Arguments to the callback are the packet, transmitting
     * net device, receiving net device, transmission time and
     * packet receipt time.
     *
     * \see class CallBackTraceSource
     * \deprecated The non-const \c Ptr<NetDevice> argument is deprecated
     * and will be changed to \c Ptr<const NetDevice> in a future release.
     */
    TracedCallback<Ptr<const Packet>, // Packet being transmitted
                   Ptr<NetDevice>,    // Transmitting NetDevice
                   Ptr<NetDevice>,    // Receiving NetDevice
                   Time,              // Amount of time to transmit the pkt
                   Time               // Last bit receive time (relative to now)
                   >
        m_txrxSnic;

    /** \brief Wire states
     *
     */
    enum WireState
    {
        /** Initializing state */
        INITIALIZING,
        /** Idle state (no transmission from NetDevice) */
        IDLE,
        /** Transmitting state (data being transmitted from NetDevice. */
        TRANSMITTING,
        /** Propagating state (data is being propagated in the channel. */
        PROPAGATING
    };

    /**
     * \brief Wire model for the SnicChannel
     */
    class Link
    {
      public:
        /** \brief Create the link, it will be in INITIALIZING state
         *
         */
        Link()
            : m_state(INITIALIZING),
              m_src(nullptr),
              m_dst(nullptr)
        {
        }

        WireState m_state;                //!< State of the link
        Ptr<SnicNetDevice> m_src; //!< First NetDevice
        Ptr<SnicNetDevice> m_dst; //!< Second NetDevice
    };

    Link m_link[N_DEVICES]; //!< Link model
};

} // namespace ns3

#endif // SNIC_CHANNEL_H
