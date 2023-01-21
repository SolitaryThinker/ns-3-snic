/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_HELPER_H
#define SNIC_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/trace-helper.h"

#include <string>


namespace ns3
{

class NetDevice;
class Node;
/* ... */

class SnicHelper
{
  public:
    SnicHelper();
    ~SnicHelper() override
    {
    }

    /**
     * Set an attribute value to be propagated to each NetDevice created by the
     * helper.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     *
     * Set these attributes on each ns3::PointToPointNetDevice created
     * by PointToPointHelper::Install
     */
    void SetDeviceAttribute(std::string name, const AttributeValue& value);

    /**
     * Set an attribute value to be propagated to each Channel created by the
     * helper.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     *
     * Set these attribute on each ns3::PointToPointChannel created
     * by PointToPointHelper::Install
     */
    void SetChannelAttribute(std::string name, const AttributeValue& value);

    /**
     * Disable flow control only if you know what you are doing. By disabling
     * flow control, this NetDevice will be sent packets even if there is no
     * room for them (such packets will be likely dropped by this NetDevice).
     * Also, any queue disc installed on this NetDevice will have no effect,
     * as every packet enqueued to the traffic control layer queue disc will
     * be immediately dequeued.
     */
    void DisableFlowControl();

    /**
     * \param node node containing snic
     * \param c a set of connected netdevices
     * \return a NetDeviceContainer for connected nodes
     *
     * We want to be able to install the same instance of NetDevice onto a
     * group of Nodes to represent multi-homed setup.
     *
     * XXX: not sure how this can be done yet. The NetDevice needs to have one
     * link leaving the group of nodes to a channel or somewhere.
     */
    NetDeviceContainer Install(Ptr<Node> node, NetDeviceContainer c);

    /**
     * \param aName Name of first node
     * \param b second node
     * \return a NetDeviceContainer for nodes
     *
     * Saves you from having to construct a temporary NodeContainer.
     */
    NetDeviceContainer Install(std::string nodeName, NetDeviceContainer c);
  private:
    /**
     * \brief Enable pcap output the indicated net device.
     *
     * NetDevice-specific implementation mechanism for hooking the trace and
     * writing to the trace file.
     *
     * \param prefix Filename prefix to use for pcap files.
     * \param nd Net device for which you want to enable tracing.
     * \param promiscuous If true capture all possible packets available at the device.
     * \param explicitFilename Treat the prefix as an explicit filename if true
     */
    void EnablePcapInternal(std::string prefix,
                            Ptr<NetDevice> nd,
                            bool promiscuous,
                            bool explicitFilename) override;

    /**
     * \brief Enable ascii trace output on the indicated net device.
     *
     * NetDevice-specific implementation mechanism for hooking the trace and
     * writing to the trace file.
     *
     * \param stream The output stream object to use when logging ascii traces.
     * \param prefix Filename prefix to use for ascii trace files.
     * \param nd Net device for which you want to enable tracing.
     * \param explicitFilename Treat the prefix as an explicit filename if true
     */
    void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                             std::string prefix,
                             Ptr<NetDevice> nd,
                             bool explicitFilename) override;

    ObjectFactory m_channelFactory; //!< Channel Factory
    ObjectFactory m_deviceFactory;  //!< Device Factory
    bool m_enableFlowControl;       //!< whether to enable flow control
};

} // namespace ns3

#endif /* SNIC_HELPER_H */
