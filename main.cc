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
	// LogComponentEnable("StreamingClientApplication", LOG_LEVEL_ALL);		
	// LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_ALL);
	// LogComponentEnable("UdpSocketImpl", LOG_LEVEL_ALL);
	LogComponentEnable("StreamingClientApplication", LOG_LEVEL_DEBUG);
	LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_DEBUG);

	CommandLine cmd;
	uint32_t fps = 30;
	uint32_t packet_size = 30;
	uint32_t packet_nip = 30;
	bool lossEnable = false;
	double lossRate = 0.01;
	uint32_t mode = 0;
	uint32_t threshold = 30;
	uint32_t bufferSize = 40;

	// cmd.AddValue (string::"attribute", string::"explanation", anytype::variable)
	cmd.AddValue("PacketSize","PacketSize", packet_size);
	cmd.AddValue("PacketNIP","Number of packets in Frame", packet_nip);
	cmd.AddValue("fps","StreamingFPS", fps);
	cmd.AddValue("lossEn","LossEnable", lossEnable);
	cmd.AddValue("lossRate","LossRate", lossRate);
	cmd.AddValue("threshold","threshold", threshold);
	cmd.AddValue("BufferSize","BufferSize", bufferSize);

	cmd.Parse(argc,argv);

    std::string datarate="10Mbps";
    std::string delay="10us";
	
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
	echoServer.SetAttribute("LossEnable", BooleanValue (lossEnable));
	echoServer.SetAttribute("LossRate",DoubleValue(lossRate));

	echoServer.SetAttribute("PacketSize",UintegerValue(packet_size));
	echoServer.SetAttribute("PacketNIP",UintegerValue(packet_nip));
	echoServer.SetAttribute("StreamingFPS",UintegerValue(fps));

	echoServer.SetAttribute("Mode",UintegerValue(mode));
    echoServer.SetAttribute("threshold",UintegerValue(threshold));
	

	ApplicationContainer serverApps(echoServer.Install(nodes.Get(1)));
	serverApps.Start(Seconds(1.0));
	serverApps.Stop(Seconds(29.0));

	ClientHelper echoClient(9);
	echoClient.SetAttribute("PacketSize",UintegerValue(packet_size));
	echoClient.SetAttribute("PacketNIP",UintegerValue(packet_nip));

    echoClient.SetAttribute("BufferSize",UintegerValue(bufferSize));

	ApplicationContainer clientApps(echoClient.Install(nodes.Get(0)));
	clientApps.Start(Seconds(0.0));
	clientApps.Stop(Seconds(30.0));

	Simulator::Run();
	Simulator::Stop(Seconds(30.0));
	Simulator::Destroy();

	return 0;
}
