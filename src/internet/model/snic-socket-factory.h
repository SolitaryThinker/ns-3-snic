/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_SOCKET_FACTORY_H
#define SNIC_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3
{

class Socket;

/**
 * \ingroup socket
 * \ingroup ipv4
 *
 * \brief API to create Snic socket instances
 *
 * This abstract class defines the API for Snic socket factory.
 *
 */
class SnicSocketFactory : public SocketFactory
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
};

} // namespace ns3

#endif /* SNIC_SOCKET_FACTORY_H */
