//This code is based on HW week8
//This code is week6_skeleton.cc

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include<iostream>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("topology1");

// static void
// CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
// {
//   std::cout << Simulator::Now ().GetSeconds () << "\t" << newCwnd << std::endl;
// }

int 
main (int argc, char *argv[])
{
    	LogComponentEnable("StreamingClientApplication", LOG_LEVEL_INFO);
	  LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_INFO);
      NS_LOG_INFO ("Start Create nodes.");
  double udpRateMbps = 0.5; // UDP source rate in Mb/s, default: 0.5 Mb/s
    uint32_t cmd_ontime=1;
    uint32_t cmd_offtime=1;
  CommandLine cmd;
  cmd.AddValue("udpRateMbps", "Datarate of UDP source in bps", udpRateMbps);

   
  int subFlowRate = udpRateMbps * 1000 * 1000; // subNode source rate in b/s
    std::string cmd_socketFactory="ns3::TcpSocketFactory";//default = tcpsocketfactory
    cmd_socketFactory="ns3::UdpSocketFactory";

    std::string cmd_ontime_string="ns3::ConstantRandomVariable[Constant="+std::to_string(cmd_ontime)+"])";
    cmd_ontime_string="ns3::ConstantRandomVariable[Constant=3])";
  
    std::string cmd_offtime_string="ns3::ConstantRandomVariable[Constant="+std::to_string(cmd_offtime)+"])";
    cmd_offtime_string="ns3::ConstantRandomVariable[Constant=3])";
  // Create nodes
  //NS_LOG_INFO ("Create nodes.");


	uint32_t fps = 30; 			//30
	uint32_t packetSize = 100; 	//100
	uint32_t packetNip = 100; 	//100
	bool lossEnable = false;	//false
	double lossRate = 0.01;	 	//0.01
	uint32_t mode = 0; 			//0
	uint32_t thresHold = 200; 	//200
	uint32_t bufferSize = 40; 	//40

	// cmd.AddValue (string::"attribute", string::"explanation", anytype::variable)
	cmd.AddValue("PacketSize","PacketSize", packetSize);
	cmd.AddValue("PacketNIP","Number of packets in Frame", packetNip);
	cmd.AddValue("Fps","StreamingFPS", fps);
	cmd.AddValue("LossEn","Forced Packet Loss on/off", lossEnable);
	cmd.AddValue("LossRate","Loss probability", lossRate);
	cmd.AddValue("Mode","Select congestion control mode", mode);
	cmd.AddValue("ThresHold","Select threshold", thresHold);
	cmd.AddValue("BufferSize","The frame buffer size", bufferSize);

	cmd.Parse(argc,argv);

  //We will use nSrc1
  Ptr<Node> nSrc1 = CreateObject<Node> ();
  Ptr<Node> nSrc2 = CreateObject<Node> ();
  Ptr<Node> nRtr = CreateObject<Node> ();
  Ptr<Node> nDst = CreateObject<Node> ();

    //nodes(0)
    //Streamer will be set on nSrc1 node.
  NodeContainer nodes = NodeContainer (nSrc1, nSrc2, nRtr, nDst);

  NodeContainer nSrc1nRtr = NodeContainer(nSrc1, nRtr);
  NodeContainer nSrc2nRtr = NodeContainer(nSrc2, nRtr);
  NodeContainer nRtrnDst  = NodeContainer(nRtr, nDst);

  InternetStackHelper stack;
  stack.Install (nodes);

  // Create P2P channels
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer dSrc1dRtr = p2p.Install (nSrc1nRtr);
  NetDeviceContainer dSrc2dRtr = p2p.Install (nSrc2nRtr);
  NetDeviceContainer dRtrdDst  = p2p.Install (nRtrnDst);

  // Add IP addresses
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iSrc1iRtr = ipv4.Assign (dSrc1dRtr);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iSrc2iRtr = ipv4.Assign (dSrc2dRtr);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iRtriDst = ipv4.Assign (dRtrdDst);

  // Set up the routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Implement TCP & UDP sinks to the destinations
  //Remove 1 sink.
  uint16_t PortStream = 8080;
  uint16_t sinkPort = 9090;
  Address sinkAddressStreamer (InetSocketAddress (iRtriDst.GetAddress (1), PortStream));//TODO
  Address sinkAddress (InetSocketAddress (iRtriDst.GetAddress (1), sinkPort));

//==========================================================================================
/* ToDo: Install packet sinks to the destinations
     Hint: Need to install packet sinks for both TCP and UDP traffic */
     //We need to choose socketFactory
     //std::string socketFactory
    //PacketSinkHelper packetSinkHelperTcp (cmd_socketFactory,InetSocketAddress(Ipv4Address::GetAny(),sinkPortStream));
   //ApplicationContainer sinkAppTcp =packetSinkHelperTcp.Install(nodes.Get(3));//TODO:: this is for streamer not for TCP!

    //we need client at here
    
    ClientHelper client(PortStream);
    ApplicationContainer clientApp(client.Install(nodes.Get(3)));
   //remove below
   PacketSinkHelper packetSinkHelper (cmd_socketFactory,InetSocketAddress(Ipv4Address::GetAny(),sinkPort));
   ApplicationContainer sinkApp =packetSinkHelper.Install(nodes.Get(3));
//==========================================================================================

  clientApp.Start (Seconds (0.));
  clientApp.Stop (Seconds (30.));
  //remove below
  sinkApp.Start (Seconds (0.));
  sinkApp.Stop (Seconds (30.));

  
  //we should change this to Streamer.
  //OnOffHelper onoffTcp("ns3::TcpSocketFactory", sinkAddressStreamer);
  //onoffTcp.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  //onoffTcp.SetAttribute("OffTime",   StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  //onoffTcp.SetAttribute("DataRate", DataRateValue(500000));
  //ApplicationContainer sourceAppTcp = onoffTcp.Install(nSrc1);
  //sourceAppTcp.Start (Seconds (5.));
  //sourceAppTcp.Stop (Seconds (20.));

    StreamerHelper streamer(iRtriDst.GetAddress(1), PortStream);//address, port num.
    streamer.SetAttribute("LossEnable", BooleanValue (lossEnable));
	streamer.SetAttribute("LossRate",DoubleValue(lossRate));

	streamer.SetAttribute("PacketSize",UintegerValue(packetSize));
	streamer.SetAttribute("PacketNIP",UintegerValue(packetNip));
	streamer.SetAttribute("StreamingFPS",UintegerValue(fps));

	streamer.SetAttribute("Mode",UintegerValue(mode));
    streamer.SetAttribute("threshold",UintegerValue(thresHold));
  //streamer.SetAttribute("DataRate", DataRateValue(5000));//TODO:: set datarate.
  ApplicationContainer streamApp(streamer.Install(nSrc1));
  streamApp.Start(Seconds(0.));
  streamApp.Stop(Seconds(30.));


//==========================================================================================
/* ToDo: Connect the trace source and the trace sink
    Hint: Refer to week6_ex4.cc */

    //TODO: understand. this is for TCP cwnd trace. We don't need this func.
  /*
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get(0),TcpSocketFactory::GetTypeId());
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow",MakeCallback(&CwndChange));
  nSrc1->GetApplication (0)->GetObject<OnOffApplication> ()->SetSocket (ns3TcpSocket);
  */
//==========================================================================================


//==========================================================================================
/* ToDo: Implement UDP application
    Hint: Refer to the TCP app implementation procedure above 
    Warning: UDP app turns on and off every 1s and use variable "udpRate" for DataRate */

    //Flow2
   OnOffHelper onoff(cmd_socketFactory,sinkAddress);
   
   onoff.SetAttribute("OnTime",StringValue(cmd_ontime_string));//ontime constant=1
   onoff.SetAttribute("OffTime",StringValue(cmd_offtime_string));//offtime constant=1
   onoff.SetAttribute("DataRate",DataRateValue(subFlowRate));//data rate
   ApplicationContainer sourceApp = onoff.Install(nodes.Get(1));
   sourceApp.Start (Seconds (1.));
   sourceApp.Stop (Seconds (30.));
//==========================================================================================

  Simulator::Stop (Seconds (30));
  Simulator::Run ();
  Simulator::Destroy ();



}