
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <iostream>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("topology3");

int main(int argc, char *argv[])
{
  LogComponentEnable("StreamingClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_INFO);


  CommandLine cmd;

  int subFlowRate = 0.5 * 1000 * 1000; // subNode source rate in b/s

  uint32_t fps = 30;         // 30
  uint32_t packetSize = 100; // 100
  uint32_t packetNip = 100;  // 100
  bool lossEnable = false;   // false
  double lossRate = 0.01;    // 0.01
  uint32_t mode = 0;         // 0
  uint32_t thresHold = 200;  // 200
  uint32_t bufferSize = 40;  // 40
  uint32_t cmd_ontime = 1;
  uint32_t cmd_offtime = 1;
  bool isTcp = 1;
  // cmd.AddValue (string::"attribute", string::"explanation", anytype::variable)
  cmd.AddValue("PacketSize", "PacketSize", packetSize);
  cmd.AddValue("PacketNIP", "Number of packets in Frame", packetNip);
  cmd.AddValue("Fps", "StreamingFPS", fps);
  cmd.AddValue("LossEn", "Forced Packet Loss on/off", lossEnable);
  cmd.AddValue("LossRate", "Loss probability", lossRate);
  cmd.AddValue("Mode", "Select congestion control mode", mode);
  cmd.AddValue("ThresHold", "Select threshold", thresHold);
  cmd.AddValue("BufferSize", "The frame buffer size", bufferSize);
  cmd.AddValue("protocol", "protocol of sub flow if Tcp  type true if Udp type false", isTcp);
  cmd.AddValue("ontime", "subflow's ontime", cmd_ontime);
  cmd.AddValue("offtime", "subflow's offtime", cmd_offtime);

  cmd.Parse(argc, argv);
  std::string cmd_socketFactory = isTcp ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory"; // default = tcpsocketfactory
  std::string cmd_ontime_string = "ns3::ConstantRandomVariable[Constant=" + std::to_string(cmd_ontime) + "])";
  std::string cmd_offtime_string = "ns3::ConstantRandomVariable[Constant=" + std::to_string(cmd_offtime) + "])";

  Ptr<Node> nSrc1 = CreateObject<Node>();
  Ptr<Node> nSrc2 = CreateObject<Node>();
  Ptr<Node> nRtr = CreateObject<Node>();
  Ptr<Node> nDst = CreateObject<Node>();

  NodeContainer nodes = NodeContainer(nSrc1, nSrc2, nRtr, nDst);

  NodeContainer nSrc1nRtr = NodeContainer(nSrc1, nRtr);
  NodeContainer nSrc2nRtr = NodeContainer(nSrc2, nRtr);
  NodeContainer nRtrnDst = NodeContainer(nRtr, nDst);

  InternetStackHelper stack;
  stack.Install(nodes);

  // Create P2P channels
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer dSrc1dRtr = p2p.Install(nSrc1nRtr);
  NetDeviceContainer dSrc2dRtr = p2p.Install(nSrc2nRtr);
  NetDeviceContainer dRtrdDst = p2p.Install(nRtrnDst);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iSrc1iRtr = ipv4.Assign(dSrc1dRtr);
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iSrc2iRtr = ipv4.Assign(dSrc2dRtr);
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iRtriDst = ipv4.Assign(dRtrdDst);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  uint16_t PortStream = 8080;
  uint16_t sinkPort = 9090;
  Address sinkAddressStreamer(InetSocketAddress(iRtriDst.GetAddress(1), PortStream)); // TODO
  Address sinkAddress(InetSocketAddress(iRtriDst.GetAddress(1), sinkPort));

  ClientHelper client(PortStream);
  client.SetAttribute("PacketSize", UintegerValue(packetSize));
  client.SetAttribute("PacketNIP", UintegerValue(packetNip));
  client.SetAttribute("BufferSize", UintegerValue(bufferSize));
  ApplicationContainer clientApp(client.Install(nodes.Get(3)));
  // remove below
  PacketSinkHelper packetSinkHelper(cmd_socketFactory, InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
  ApplicationContainer sinkApp = packetSinkHelper.Install(nodes.Get(3));
  //==========================================================================================

  clientApp.Start(Seconds(0.));
  clientApp.Stop(Seconds(30.));
  // remove below
  sinkApp.Start(Seconds(0.));
  sinkApp.Stop(Seconds(30.));

  StreamerHelper streamer(iRtriDst.GetAddress(1), PortStream); // address, port num.
  streamer.SetAttribute("LossEnable", BooleanValue(lossEnable));
  streamer.SetAttribute("LossRate", DoubleValue(lossRate));
  streamer.SetAttribute("PacketSize", UintegerValue(packetSize));
  streamer.SetAttribute("PacketNIP", UintegerValue(packetNip));
  streamer.SetAttribute("StreamingFPS", UintegerValue(fps));
  streamer.SetAttribute("Mode", UintegerValue(mode));
  streamer.SetAttribute("threshold", UintegerValue(thresHold));
  // streamer.SetAttribute("DataRate", DataRateValue(5000));//TODO:: set datarate.
  ApplicationContainer streamApp(streamer.Install(nSrc1));
  streamApp.Start(Seconds(0.));
  streamApp.Stop(Seconds(30.));

  // Flow2
  OnOffHelper onoff(cmd_socketFactory, sinkAddress);

  onoff.SetAttribute("OnTime", StringValue(cmd_ontime_string));   // ontime constant=1
  onoff.SetAttribute("OffTime", StringValue(cmd_offtime_string)); // offtime constant=1
  onoff.SetAttribute("DataRate", DataRateValue(subFlowRate));     // data rate
  ApplicationContainer sourceApp = onoff.Install(nodes.Get(1));
  sourceApp.Start(Seconds(1.));
  sourceApp.Stop(Seconds(30.));
  //==========================================================================================

  Simulator::Stop(Seconds(30));
  Simulator::Run();
  Simulator::Destroy();
}