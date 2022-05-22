#include "helper.h"
#include "streamer.h"
#include "client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {


// Clinet
ClientHelper::ClientHelper (Address address, uint16_t port)
{
	m_factory.SetTypeId (Client::GetTypeId());
	SetAttribute ("RemoteAddress", AddressValue (address));
	SetAttribute ("RemotePort", UintegerValue (port));
}

void
ClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
	m_factory.Set (name, value);
}

ApplicationContainer
ClientHelper::Install (Ptr<Node> node) const
{
	return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
ClientHelper::InstallPriv (Ptr<Node> node) const
{
	Ptr<Application> app = m_factory.Create<Client> ();
	node->AddApplication (app);
	
	return app;
}


// Streamer
StreamerHelper::StreamerHelper (Address address, uint16_t port)
{
	m_factory.SetTypeId (Streamer::GetTypeId ());
	SetAttribute ("RemoteAddress", AddressValue (address));
	SetAttribute ("RemotePort", UintegerValue (port));
}

void 
StreamerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
	m_factory.Set (name, value);
}

ApplicationContainer
StreamerHelper::Install (Ptr<Node> node) const
{
	return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
StreamerHelper::InstallPriv (Ptr<Node> node) const
{
	Ptr<Application> app = m_factory.Create<Streamer> ();
	node->AddApplication (app);

	return app;
}

}

