/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("topology4");

int main(int argc, char *argv[])
{

    LogComponentEnable("StreamingClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_INFO);

    CommandLine cmd;

    uint32_t fps = 30;         // 30
    uint32_t packetSize = 100; // 100
    uint32_t packetNip = 100;  // 100
    bool lossEnable = true;   // true
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
    //
    NS_LOG_INFO("Create nodes.");
    NodeContainer nodes;
    nodes.Create(4);

    NS_LOG_INFO("Build Topology");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer devices = csma.Install(nodes);

    InternetStackHelper internet;
    internet.Install(nodes);

    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    NS_LOG_INFO("Create Applications.");
    uint16_t port = 9;

    OnOffHelper onoff(cmd_socketFactory,
                      Address(InetSocketAddress(interfaces.GetAddress(1), port)));
    onoff.SetAttribute("OnTime", StringValue(cmd_ontime_string));   // ontime constant=1
    onoff.SetAttribute("OffTime", StringValue(cmd_offtime_string)); // offtime constant=1
    onoff.SetConstantRate(DataRate("500kb/s"));

    ApplicationContainer app = onoff.Install(nodes.Get(0));

    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));

    PacketSinkHelper sink(cmd_socketFactory,
                          Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    app = sink.Install(nodes.Get(1));
    app.Start(Seconds(0.0));

    onoff.SetAttribute("Remote",
                       AddressValue(InetSocketAddress(interfaces.GetAddress(0), port)));

    StreamerHelper streamer(interfaces.GetAddress(3), 9);
    streamer.SetAttribute("LossEnable", BooleanValue(lossEnable));
    streamer.SetAttribute("LossRate", DoubleValue(lossRate));
    streamer.SetAttribute("PacketSize", UintegerValue(packetSize));
    streamer.SetAttribute("PacketNIP", UintegerValue(packetNip));
    streamer.SetAttribute("StreamingFPS", UintegerValue(fps));
    streamer.SetAttribute("Mode", UintegerValue(mode));
    streamer.SetAttribute("threshold", UintegerValue(thresHold));

    ApplicationContainer streamerapp(streamer.Install(nodes.Get(2)));

    ClientHelper client(9);
    client.SetAttribute("PacketSize", UintegerValue(packetSize));
    client.SetAttribute("PacketNIP", UintegerValue(packetNip));
    client.SetAttribute("BufferSize", UintegerValue(bufferSize));

    ApplicationContainer clientapp(client.Install(nodes.Get(3)));

    streamerapp.Start(Seconds(0.));
    streamerapp.Stop(Seconds(30.));

    clientapp.Start(Seconds(0.));
    clientapp.Stop(Seconds(30.));
    NS_LOG_INFO("Configure Tracing.");

    AsciiTraceHelper ascii;
    csma.EnableAsciiAll(ascii.CreateFileStream("csma-one-subnet.tr"));

    csma.EnablePcapAll("csma-one-subnet", false);

    Simulator::Run();
    Simulator::Stop(Seconds(30.0));
    Simulator::Destroy();
}
