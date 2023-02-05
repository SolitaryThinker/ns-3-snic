/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_NETWORK_TASK_H
#define SNIC_NETWORK_TASK_H

namespace ns3
{

class NetworkTask : public Object
{
  public:
    NetworkTask();
    ~NetworkTask();

    virtual void ProcessPacket(Ptr<Packet> packet) = 0;
    // private:
};

} // namespace ns3

#endif // SNIC_NETWORK_TASK_H
