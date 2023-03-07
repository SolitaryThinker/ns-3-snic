#ifndef SNIC_WORKLOAD_SERVER_H
#define SNIC_WORKLOAD_SERVER_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <queue>

namespace ns3
{

class Socket;
class Packet;

/**
 * \ingroup applications
 * \defgroup udpecho UdpEcho
 */

/**
 * \ingroup udpecho
 * \brief A Udp Echo server
 *
 * Every packet received is sent back.
 */
class SnicWorkloadServer : public Application
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    SnicWorkloadServer();
    ~SnicWorkloadServer() override;
    void Reset();

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * \brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * \param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);

    uint16_t m_port;       //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;  //!< IPv4 Socket
    Ptr<Socket> m_socket6; //!< IPv6 Socket
    Address m_local;       //!< local multicast address
    // tmp stat variables
    uint64_t m_numReceived = 0;
    std::vector<uint32_t> m_uids;
    Time m_avgInterval;
    Time m_avgTotal;
    Time m_lastPacket;
    Time m_last5Packets;
    std::queue<Time> m_lastTimes;

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* SNIC_WORKLOAD_SERVER_H */
