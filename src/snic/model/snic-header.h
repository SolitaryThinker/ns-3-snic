/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_HEADER_H
#define SNIC_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

#include <stdint.h>
#include <string>

namespace ns3
{

class SnicHeader : public Header
{
  public:
    SnicHeader();
    ~SnicHeader() override;

    /**
     * \brief Enable checksum calculation for SNIC
     */
    void EnableChecksums();
    /**
     * \param port the destination port for this SnicHeader
     */
    void SetDestinationPort(uint16_t port);
    /**
     * \param port The source port for this SnicHeader
     */
    void SetSourcePort(uint16_t port);
    /**
     * \return The source port for this SnicHeader
     */
    uint16_t GetSourcePort() const;
    /**
     * \return the destination port for this SnicHeader
     */
    uint16_t GetDestinationPort() const;

    /**
     * \param source the ip source to use in the underlying
     *        ip packet.
     * \param destination the ip destination to use in the
     *        underlying ip packet.
     * \param protocol the protocol number to use in the underlying
     *        ip packet.
     *
     * If you want to use snic checksums, you should call this
     * method prior to adding the header to a packet.
     */
    void InitializeChecksum(Address source, Address destination, uint8_t protocol);

    /**
     * \param source the ip source to use in the underlying
     *        ip packet.
     * \param destination the ip destination to use in the
     *        underlying ip packet.
     * \param protocol the protocol number to use in the underlying
     *        ip packet.
     *
     * If you want to use snic checksums, you should call this
     * method prior to adding the header to a packet.
     */
    void InitializeChecksum(Ipv4Address source, Ipv4Address destination, uint8_t protocol);

    /**
     * \param source the ip source to use in the underlying
     *        ip packet.
     * \param destination the ip destination to use in the
     *        underlying ip packet.
     * \param protocol the protocol number to use in the underlying
     *        ip packet.
     *
     * If you want to use snic checksums, you should call this
     * method prior to adding the header to a packet.
     */
    void InitializeChecksum(Ipv6Address source, Ipv6Address destination, uint8_t protocol);

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;

    /**
     * \brief Is the SNIC checksum correct ?
     * \returns true if the checksum is correct, false otherwise.
     */
    bool IsChecksumOk() const;

    /**
     * \brief Force the SNIC checksum to a given value.
     *
     * This might be useful for test purposes or to
     * restore the SNIC checksum when the SNIC header
     * has been compressed (e.g., in 6LoWPAN).
     * Note that, normally, the header checksum is
     * calculated on the fly when the packet is
     * serialized.
     *
     * When this option is used, the SNIC checksum is written in
     * the header, regardless of the global ChecksumEnabled option.
     *
     * \note The checksum value must be a big endian number.
     *
     * \param checksum the checksum to use (big endian).
     */
    void ForceChecksum(uint16_t checksum);

    /**
     * \brief Force the SNIC payload length to a given value.
     *
     * This might be useful when forging a packet for test
     * purposes.
     *
     * \param payloadSize the payload length to use.
     */
    void ForcePayloadSize(uint16_t payloadSize);

    /**
     * \brief Return the checksum (only known after a Deserialize)
     * \return The checksum for this SnicHeader
     */
    uint16_t GetChecksum();

  private:
    bool m_isOffloaded;
    /**
     * \brief Calculate the header checksum
     * \param size packet size
     * \returns the checksum
     */
    uint16_t CalculateHeaderChecksum(uint16_t size) const;
    uint16_t m_sourcePort;      //!< Source port
    uint16_t m_destinationPort; //!< Destination port
    uint16_t m_payloadSize;     //!< Payload size

    Address m_source;      //!< Source IP address
    Address m_destination; //!< Destination IP address
    uint8_t m_protocol;    //!< Protocol number
    uint16_t m_checksum;   //!< Forced Checksum value
    bool m_calcChecksum;   //!< Flag to calculate checksum
    bool m_goodChecksum;   //!< Flag to indicate that checksum is correct
};

} // namespace ns3
#endif // SNIC_HEADER_H
