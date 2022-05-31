#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <iostream>
using namespace ns3;
NS_LOG_COMPONENT_DEFINE("topology1");
int main(int argc, char *argv[])
{

	LogComponentEnable("StreamingClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_INFO);

	CommandLine cmd;
	uint32_t fps = 30;		   // 30
	uint32_t packetSize = 100; // 100
	uint32_t packetNip = 100;  // 100
	bool lossEnable = false;   // false
	double lossRate = 0.01;	   // 0.01
	uint32_t mode = 0;		   // 0
	uint32_t thresHold = 200;  // 200
	uint32_t bufferSize = 40;  // 40

	// cmd.AddValue (string::"attribute", string::"explanation", anytype::variable)
	cmd.AddValue("PacketSize", "PacketSize", packetSize);
	cmd.AddValue("PacketNIP", "Number of packets in Frame", packetNip);
	cmd.AddValue("Fps", "StreamingFPS", fps);
	cmd.AddValue("LossEn", "Forced Packet Loss on/off", lossEnable);
	cmd.AddValue("LossRate", "Loss probability", lossRate);
	cmd.AddValue("Mode", "Select congestion control mode", mode);
	cmd.AddValue("ThresHold", "Select threshold", thresHold);
	cmd.AddValue("BufferSize", "The frame buffer size", bufferSize);

	cmd.Parse(argc, argv);

	std::string datarate = "10Mbps";
	std::string delay = "10us";

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

	StreamerHelper streamer(interfaces.GetAddress(0), 9);
	streamer.SetAttribute("LossEnable", BooleanValue(lossEnable));
	streamer.SetAttribute("LossRate", DoubleValue(lossRate));
	streamer.SetAttribute("PacketSize", UintegerValue(packetSize));
	streamer.SetAttribute("PacketNIP", UintegerValue(packetNip));
	streamer.SetAttribute("StreamingFPS", UintegerValue(fps));
	streamer.SetAttribute("Mode", UintegerValue(mode));
	streamer.SetAttribute("threshold", UintegerValue(thresHold));

	ApplicationContainer streamerApps(streamer.Install(nodes.Get(1)));
	streamerApps.Start(Seconds(1.0));
	streamerApps.Stop(Seconds(29.0));

	ClientHelper client(9);
	client.SetAttribute("PacketSize", UintegerValue(packetSize));
	client.SetAttribute("PacketNIP", UintegerValue(packetNip));
	client.SetAttribute("BufferSize", UintegerValue(bufferSize));

	ApplicationContainer clientApps(client.Install(nodes.Get(0)));
	clientApps.Start(Seconds(0.0));
	clientApps.Stop(Seconds(30.0));

	Simulator::Run();
	Simulator::Stop(Seconds(30.0));
	Simulator::Destroy();

	return 0;
}
