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

class SnicRte : public Header
{
  public:
    SnicRte();

    /**
     * \brief Get the type ID.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * \brief Return the instance type identifier.
     * \return Instance type ID.
     */
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;

    /**
     * \brief Get the serialized size of the packet.
     * \return Size.
     */
    uint32_t GetSerializedSize() const override;

    /**
     * \brief Serialize the packet.
     * \param start Buffer iterator.
     */
    void Serialize(Buffer::Iterator start) const override;

    /**
     * \brief Deserialize the packet.
     * \param start Buffer iterator.
     * \return Size of the packet.
     */
    uint32_t Deserialize(Buffer::Iterator start) override;

    /**
     * \brief Set the prefix.
     * \param prefix The prefix.
     */
    void SetPrefix(Ipv4Address prefix);

    /**
     * \brief Get the prefix.
     * \returns The prefix.
     */
    Ipv4Address GetPrefix() const;

    /**
     * \brief Set the subnet mask.
     * \param subnetMask The subnet mask.
     */
    void SetSubnetMask(Ipv4Mask subnetMask);

    /**
     * \brief Get the subnet mask.
     * \returns The subnet mask.
     */
    Ipv4Mask GetSubnetMask() const;

    /**
     * \brief Set the route tag.
     * \param routeTag The route tag.
     */
    void SetRouteTag(uint16_t routeTag);

    /**
     * \brief Get the route tag.
     * \returns The route tag.
     */
    uint16_t GetRouteTag() const;

    /**
     * \brief Set the route metric.
     * \param routeMetric The route metric.
     */
    void SetRouteMetric(uint32_t routeMetric);

    /**
     * \brief Get the route metric.
     * \returns The route metric.
     */
    uint32_t GetRouteMetric() const;

    /**
     * \brief Set the next hop.
     * \param nextHop The next hop.
     */
    void SetNextHop(Ipv4Address nextHop);

    /**
     * \brief Get the next hop.
     * \returns The next hop.
     */
    Ipv4Address GetNextHop() const;

  private:
    uint16_t m_tag;        //!< Route tag.
    Ipv4Address m_prefix;  //!< Advertised prefix.
    Ipv4Mask m_subnetMask; //!< Subnet mask.
    Ipv4Address m_nextHop; //!< Next hop.
    uint32_t m_metric;     //!< Route metric.
};

class SnicHeader : public Header
{
  public:
    SnicHeader();
    ~SnicHeader() override;

    void AddNT(uint64_t nt);

    /* returns 0 if no NT */
    uint64_t GetNT();

    // XXX
    void SetPayload(uint8_t* buffer, size_t size);
    void CopyPayload(uint8_t* buffer, size_t size) const;

    bool HasSeenNic() const;
    void SetHasSeenNic();

    uint16_t GetPacketType() const;
    void SetPacketType(uint16_t packetType);

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

    void SetSourceIp(Address ip);
    void SetDestinationIp(Address ip);
    Address GetSourceIp() const;
    Address GetDestinationIp() const;
    void SetProtocol(uint8_t protocol);
    uint8_t GetProtocol() const;

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

    enum SnicPacketType_e
    {
        L4_PACKET = 0,
        ALLOCATION_REQUEST = 1,
        ALLOCATION_RESPONSE = 2,
        ALLOCATION_RELEASE = 3,
        RECONFIG_REQUEST,
        RECONFIG_RESPONSE
    };

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
    uint16_t m_nt;
    int64_t m_payload;
    bool m_hasSeenNic;
    uint16_t m_packetType; // anything other than 0 means this packet is internal to snic cluster
    uint16_t m_payloadSize;     //!< Payload size

    Address m_source;      //!< Source IP address
    Address m_destination; //!< Destination IP address
    uint8_t m_protocol;    //!< Protocol number
    uint16_t m_checksum;   //!< Forced Checksum value
    bool m_calcChecksum;   //!< Flag to calculate checksum
    bool m_goodChecksum;   //!< Flag to indicate that checksum is correct
    std::list<SnicRte> m_rteList;
};

} // namespace ns3
#endif // SNIC_HEADER_H
