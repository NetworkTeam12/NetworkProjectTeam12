#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "streamer.h"
#include "client.h"
#include "helper.h"

#include <iostream>
using namespace ns3;

//NS_LOG_COMPONENT_DEFINE("main_project");

int 
main(int argc, char*argv[])
{
    /*
    Topol2.
        
        N1 - N2 - N3 - N4   1. 각 노드간 활발한 송수신.
    */
    std::string datarate="10Mbps";
    std::string delay="10us";

    // logs
   //LogComponentEnable("main_project", LOG_LEVEL_ALL);
   
    // node container
   NodeContainer nodes;
   nodes.Create(4);
   
   

   NodeContainer n0n1 = NodeContainer(nodes.Get(0), nodes.Get(1));
   NodeContainer n1n2 = NodeContainer(nodes.Get(1), nodes.Get(2));
   NodeContainer n2n3 = NodeContainer(nodes.Get(2), nodes.Get(3));
   
    

    // connect setting
   PointToPointHelper p2p_1, p2p_2,p2p_3;
   
    p2p_1.SetDeviceAttribute("DataRate", StringValue(datarate));
    p2p_1.SetChannelAttribute("Delay", StringValue(delay));

    p2p_2.SetDeviceAttribute("DataRate", StringValue(datarate));
    p2p_2.SetChannelAttribute("Delay", StringValue(delay));

    p2p_3.SetDeviceAttribute("DataRate", StringValue(datarate));
    p2p_3.SetChannelAttribute("Delay", StringValue(delay));


    // nic ethernet address
    NetDeviceContainer devices1, devices2, devices3;
    devices1 = p2p_1.Install(n0n1);
    devices2 = p2p_2.Install(n1n2);
    devices3 = p2p_3.Install(n2n3);

    // enable pcap file
    // p2p_1.EnablePcapAll("main_project");
    
   InternetStackHelper stack;
   stack.Install(nodes);

   Ipv4AddressHelper addr1, addr2,addr3;
   addr1.SetBase("10.1.1.0", "255.255.255.0");
   Ipv4InterfaceContainer interfaces1 = addr1.Assign(devices1); // 1.1.1, 2
   Ipv4InterfaceContainer interfaces2 = addr2.Assign(devices2);
   Ipv4InterfaceContainer interfaces3 = addr3.Assign(devices3);

   



//========================================
    //Appl. abstaction.
//========================================
    
    Simulator::Run();
   Simulator::Stop(Seconds(11.0));
   Simulator::Destroy();

   return 0;
}


/*
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "streamer.h"
#include "client.h"
#include "helper.h"

#include <iostream>
using namespace ns3;

//NS_LOG_COMPONENT_DEFINE("main_project");

int 
main(int argc, char*argv[])
{
    std::string datarate="10Mbps";
    std::string delay="10us";

    // logs
   //LogComponentEnable("main_project", LOG_LEVEL_ALL);
   
    // node container
   NodeContainer nodes;
   nodes.Create(2);
    NodeContainer n0n1 = NodeContainer(nodes.Get(0), nodes.Get(1));
    

    // connect setting
   PointToPointHelper p2p_1, p2p_2;
   
    p2p_1.SetDeviceAttribute("DataRate", StringValue(datarate));
    p2p_1.SetChannelAttribute("Delay", StringValue(delay));

    // nic ethernet address
   NetDeviceContainer devices1, devices2;
   devices1 = p2p_1.Install(n0n1);

    // enable pcap file
    // p2p_1.EnablePcapAll("main_project");
    
   InternetStackHelper stack;
   stack.Install(nodes);

   Ipv4AddressHelper addr1, addr2;
   addr1.SetBase("10.1.1.0", "255.255.255.0");
   Ipv4InterfaceContainer interfaces1 = addr1.Assign(devices1); // 1.1.1, 2
   



//========================================
   // Client
   ClientHelper Client1(interfaces1.GetAddress(1), 9);
   ApplicationContainer clientApps1;
   clientApps1.Add(Client1.Install(nodes.Get(0)));
   clientApps1.Start(Seconds(1.0));
   clientApps1.Stop(Seconds(10.0));
    
    // Server
    StreamerHelper Server1(interfaces1.GetAddress(0),9);
   ApplicationContainer serverApps1(Server1.Install(nodes.Get(1)));
   serverApps1.Start(Seconds(0));
   serverApps1.Stop(Seconds(11.0));
//========================================
    
    Simulator::Run();
   Simulator::Stop(Seconds(11.0));
   Simulator::Destroy();

   return 0;
}

*/