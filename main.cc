#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <iostream>
using namespace ns3;

int 
main(int argc, char*argv[])
{
	LogComponentEnable("StreamingClientApplication", LOG_LEVEL_ALL);		
	LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_ALL);

	NodeContainer nodes;
	nodes.Create(2);

	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
	p2p.SetChannelAttribute("Delay", StringValue("2ms"));

	NetDeviceContainer devices;
	devices = p2p.Install(nodes);

	InternetStackHelper stack;
	stack.Install(nodes);

	Ipv4AddressHelper addr;
	addr.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = addr.Assign(devices);
	
    	StreamerHelper echoServer(interfaces.GetAddress(0), 9);
    	echoServer.SetAttribute("LossEnable", BooleanValue (true));
	ApplicationContainer serverApps(echoServer.Install(nodes.Get(1)));
	serverApps.Start(Seconds(1.0));
	serverApps.Stop(Seconds(3.0));

	ClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("LossEnable", BooleanValue (true));
	ApplicationContainer clientApps(echoClient.Install(nodes.Get(0)));
	clientApps.Start(Seconds(0.0));
	clientApps.Stop(Seconds(4.0));

	
	Simulator::Run();
	Simulator::Stop(Seconds(4.0));
	Simulator::Destroy();

	return 0;
}
