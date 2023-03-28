/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_SCHEDULER_HEADER_H
#define SNIC_SCHEDULER_HEADER_H

#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/snic-header.h"

#include <stdint.h>
#include <string>

namespace ns3
{

class SnicSchedulerHeader : public Header
{
  public:
    SnicSchedulerHeader();
    SnicSchedulerHeader(Ipv4Address srcIp,
                        uint16_t srcPort,
                        Ipv4Address dstIp,
                        uint16_t dstPort,
                        uint8_t protocol,
                        uint64_t flowId);
    SnicSchedulerHeader(Ipv4Header ipv4Header, SnicHeader snicHeader);
    ~SnicSchedulerHeader() override;

    void SetBandwidthDemand(double demand);
    double GetBandwidthDemand() const;

    void SetResourceDemand(uint32_t demand);
    uint32_t GetResourceDemand() const;

    void SetNumNetworkTask(uint16_t num);
    uint16_t GetNumNetworkTask() const;

    void AddNT(uint64_t nt);

    /* returns 0 if no NT */
    uint64_t GetNT();

    uint16_t GetPacketType() const;
    void SetPacketType(uint16_t packetType);

    /**
     * \param port the destination port for this SnicSchedulerHeader
     */
    void SetDestinationPort(uint16_t port);
    /**
     * \param port The source port for this SnicSchedulerHeader
     */
    void SetSourcePort(uint16_t port);
    /**
     * \return The source port for this SnicSchedulerHeader
     */
    uint16_t GetSourcePort() const;
    /**
     * \return the destination port for this SnicSchedulerHeader
     */
    uint16_t GetDestinationPort() const;

    void SetSourceIp(Ipv4Address ip);
    void SetDestinationIp(Ipv4Address ip);
    Ipv4Address GetSourceIp() const;
    Ipv4Address GetDestinationIp() const;
    void SetProtocol(uint8_t protocol);
    uint8_t GetProtocol() const;

    void SetFlowId(uint64_t flowId);
    uint64_t GetFlowId() const;

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

    enum SnicPacketType_e
    {
        L4_PACKET = 0,
        ALLOCATION_REQUEST = 1,
        ALLOCATION_RESPONSE = 2,
        ALLOCATION_RELEASE = 3
    };

  private:
    // bool m_isOffloaded;
    /**
     * \brief Calculate the header checksum
     * \param size packet size
     * \returns the checksum
     */
    double m_bandwidthDemand;
    uint32_t m_resourceDemand;
    uint16_t m_numNetworkTask;
    uint16_t m_nt;

    uint16_t m_sourcePort;      //!< Source port
    uint16_t m_destinationPort; //!< Destination port
    uint16_t m_packetType;  // anything other than 0 means this packet is internal to snic cluster

    Ipv4Address m_source;      //!< Source IP address
    Ipv4Address m_destination; //!< Destination IP address
    uint8_t m_protocol;    //!< Protocol number
    uint64_t m_flowId;
};

} // namespace ns3
#endif // SNIC_SCHEDULER_HEADER_H
