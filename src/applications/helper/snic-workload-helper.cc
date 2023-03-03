#include "snic-workload-helper.h"

#include "ns3/names.h"
#include "ns3/snic-workload-client.h"
#include "ns3/snic-workload-server.h"
#include "ns3/uinteger.h"

namespace ns3
{

SnicWorkloadServerHelper::SnicWorkloadServerHelper(uint16_t port)
{
    m_factory.SetTypeId(SnicWorkloadServer::GetTypeId());
    SetAttribute("Port", UintegerValue(port));
}

void
SnicWorkloadServerHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
SnicWorkloadServerHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
SnicWorkloadServerHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
SnicWorkloadServerHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
SnicWorkloadServerHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<SnicWorkloadServer>();
    node->AddApplication(app);

    return app;
}

SnicWorkloadClientHelper::SnicWorkloadClientHelper(Address address, uint16_t port)
{
    m_factory.SetTypeId(SnicWorkloadClient::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
}

SnicWorkloadClientHelper::SnicWorkloadClientHelper(Address address)
{
    m_factory.SetTypeId(SnicWorkloadClient::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
}

void
SnicWorkloadClientHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

void
SnicWorkloadClientHelper::SetFill(Ptr<Application> app, std::string fill)
{
    app->GetObject<SnicWorkloadClient>()->SetFill(fill);
}

void
SnicWorkloadClientHelper::SetFill(Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
    app->GetObject<SnicWorkloadClient>()->SetFill(fill, dataLength);
}

void
SnicWorkloadClientHelper::SetFill(Ptr<Application> app,
                                  uint8_t* fill,
                                  uint32_t fillLength,
                                  uint32_t dataLength)
{
    app->GetObject<SnicWorkloadClient>()->SetFill(fill, fillLength, dataLength);
}

ApplicationContainer
SnicWorkloadClientHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
SnicWorkloadClientHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
SnicWorkloadClientHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
SnicWorkloadClientHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<SnicWorkloadClient>();
    node->AddApplication(app);

    return app;
}

} // namespace ns3
