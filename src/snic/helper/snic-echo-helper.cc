#include "snic-echo-helper.h"

#include "ns3/names.h"
#include "ns3/snic-echo-client.h"
#include "ns3/snic-echo-server.h"
#include "ns3/uinteger.h"

namespace ns3
{

SnicEchoServerHelper::SnicEchoServerHelper(uint16_t port)
{
    m_factory.SetTypeId(SnicEchoServer::GetTypeId());
    SetAttribute("Port", UintegerValue(port));
}

void
SnicEchoServerHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
SnicEchoServerHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
SnicEchoServerHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
SnicEchoServerHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
SnicEchoServerHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<SnicEchoServer>();
    node->AddApplication(app);

    return app;
}

SnicEchoClientHelper::SnicEchoClientHelper(Address address, uint16_t port)
{
    m_factory.SetTypeId(SnicEchoClient::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
}

SnicEchoClientHelper::SnicEchoClientHelper(Address address)
{
    m_factory.SetTypeId(SnicEchoClient::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
}

void
SnicEchoClientHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

void
SnicEchoClientHelper::SetFill(Ptr<Application> app, std::string fill)
{
    app->GetObject<SnicEchoClient>()->SetFill(fill);
}

void
SnicEchoClientHelper::SetFill(Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
    app->GetObject<SnicEchoClient>()->SetFill(fill, dataLength);
}

void
SnicEchoClientHelper::SetFill(Ptr<Application> app,
                              uint8_t* fill,
                              uint32_t fillLength,
                              uint32_t dataLength)
{
    app->GetObject<SnicEchoClient>()->SetFill(fill, fillLength, dataLength);
}

ApplicationContainer
SnicEchoClientHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
SnicEchoClientHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
SnicEchoClientHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
SnicEchoClientHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<SnicEchoClient>();
    node->AddApplication(app);

    return app;
}

} // namespace ns3
