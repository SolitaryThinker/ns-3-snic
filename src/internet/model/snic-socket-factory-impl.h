#ifndef SNIC_SOCKET_FACTORY_IMPL_H
#define SNIC_SOCKET_FACTORY_IMPL_H

#include "ns3/ptr.h"
#include "ns3/snic-socket-factory.h"

namespace ns3
{

/**
 * \ingroup socket
 * \ingroup ipv4
 *
 * \brief Implementation of IPv4 snic socket factory.
 */
class SnicL4Protocol;

/**
 * \ingroup socket
 * \ingroup udp
 *
 * \brief Object to create UDP socket instances
 *
 * This class implements the API for creating UDP sockets.
 * It is a socket factory (deriving from class SocketFactory).
 */
class SnicSocketFactoryImpl : public SnicSocketFactory
{
  public:
    SnicSocketFactoryImpl();
    ~SnicSocketFactoryImpl() override;

    /**
     * \brief Set the associated UDP L4 protocol.
     * \param udp the UDP L4 protocol
     */
    void SetSnic(Ptr<SnicL4Protocol> snic);

    /**
     * \brief Implements a method to create a Udp-based socket and return
     * a base class smart pointer to the socket.
     *
     * \return smart pointer to Socket
     */
    Ptr<Socket> CreateSocket() override;

  protected:
    void DoDispose() override;

  private:
    Ptr<SnicL4Protocol> m_snic; //!< the associated UDP L4 protocol
};

} // namespace ns3

#endif /* SNIC_SOCKET_FACTORY_IMPL_H */
