#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include <iostream>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("topology2");

int main(int argc, char **argv)
{

    LogComponentEnable("StreamingClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    uint32_t payloadSize = 1472;
    uint64_t simulationTime = 10;
    CommandLine cmd;

    uint32_t fps = 30;         // 30
    uint32_t packetSize = 100; // 100
    uint32_t packetNip = 100;  // 100
    bool lossEnable = true;   // true
    double lossRate = 0.01;    // 0.01
    uint32_t mode = 0;         // 0
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

    // 1. Create Nodes : Make 1 STA and 1 AP
    NodeContainer wifiStaNode;
    wifiStaNode.Create(1);
    NodeContainer wifiApNode;
    wifiApNode.Create(1);

    // 2.Create PHY layer (Wireless channel)
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    phy.SetChannel(channel.Create());

    // 3. Creata Mac layer
    WifiMacHelper mac;
    Ssid ssid = Ssid("Week8Example");
    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false));

    // 4. Create WLAN setting
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("HtMcs7"),
                                 "ControlMode", StringValue("HtMcs0"));

    // 5. Create NetDevices
    NetDeviceContainer staDevice;
    staDevice = wifi.Install(phy, mac, wifiStaNode);

    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid),
                "BeaconInterval", TimeValue(MicroSeconds(102400)),
                "BeaconGeneration", BooleanValue(true));

    NetDeviceContainer apDevice;
    apDevice = wifi.Install(phy, mac, wifiApNode);

    // 6. Create Network layer
    /*Internet stack*/
    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(wifiStaNode);

    Ipv4AddressHelper address;

    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer StaInterface;
    StaInterface = address.Assign(staDevice);
    Ipv4InterfaceContainer ApInterface;
    ApInterface = address.Assign(apDevice);

    // 7.Locate nodes
    /* Settin mobility model*/
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    positionAlloc->Add(Vector(0.0, 0.0, 0.0)); // Add vector x=0, y=0, z=0
    positionAlloc->Add(Vector(1.0, 0.0, 0.0)); // Add vector x=1, y=0, z=0
    mobility.SetPositionAllocator(positionAlloc);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.Install(wifiApNode);
    mobility.Install(wifiStaNode);

    // 8. Create Transport layer (Udp)

    ClientHelper client(10);
    client.SetAttribute("PacketSize", UintegerValue(packetSize));
    client.SetAttribute("PacketNIP", UintegerValue(packetNip));
    client.SetAttribute("BufferSize", UintegerValue(bufferSize));

    ApplicationContainer clientApp(client.Install(wifiStaNode.Get(0)));
    clientApp.Start(Seconds(0.0));
    clientApp.Stop(Seconds(simulationTime + 1));


    StreamerHelper streamer(StaInterface.GetAddress(0), 10);
    streamer.SetAttribute("LossEnable", BooleanValue(lossEnable));
    streamer.SetAttribute("LossRate", DoubleValue(lossRate));
    streamer.SetAttribute("PacketSize", UintegerValue(packetSize));
    streamer.SetAttribute("PacketNIP", UintegerValue(packetNip));
    streamer.SetAttribute("StreamingFPS", UintegerValue(fps));
    streamer.SetAttribute("Mode", UintegerValue(mode));
    streamer.SetAttribute("threshold", UintegerValue(thresHold));

    // ApplicationContainer clientApp = myClient.Install(wifiApNode.Get(0));
    ApplicationContainer streamerApp(streamer.Install(wifiApNode.Get(0)));
    streamerApp.Start(Seconds(1.0));
    streamerApp.Stop(Seconds(simulationTime + 1));

    // 9. Simulation Run and Calc. throughput
    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    Simulator::Destroy();

    uint32_t totalPacketRecv = DynamicCast<UdpServer>(clientApp.Get(0))->GetReceived();
    double throughput = totalPacketRecv * payloadSize * 8 / (simulationTime * 1000000.0);
    std::cout << "Throughput: " << throughput << " Mbps" << '\n';
    return 0;
}
