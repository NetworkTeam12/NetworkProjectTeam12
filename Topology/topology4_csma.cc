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

// Network topology
//
//       n0    n1   n2   n3
//       |     |    |    |
//       =================
//              LAN
//
// - CBR/UDP flows from n0 to n1 and from n3 to n0
// - DropTail queues
// - Tracing of queues and packet receptions to file "csma-one-subnet.tr"

#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("topology3");

int main(int argc, char *argv[])
{
    //
    // Users may find it convenient to turn on explicit debugging
    // for selected modules; the below lines suggest how to do this
    //
    LogComponentEnable("StreamingClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("StreamingStreamerApplication", LOG_LEVEL_INFO);
    //
    // Allow the user to override any of the defaults and the above Bind() at
    // run-time, via command-line arguments
    //
    CommandLine cmd;

    uint32_t fps = 30;         // 30
    uint32_t packetSize = 100; // 100
    uint32_t packetNip = 100;  // 100
    bool lossEnable = false;   // false
    double lossRate = 0.01;    // 0.01
    uint32_t mode = 0;         // 0
    uint32_t thresHold = 200;  // 200
    uint32_t bufferSize = 40;  // 40
    std::string cmd_socketFactory = "ns3::TcpSocketFactory"; // default = tcpsocketfactory
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
    //
    // Explicitly create the nodes required by the topology (shown above).
    //
    NS_LOG_INFO("Create nodes.");
    NodeContainer nodes;
    nodes.Create(4);

    NS_LOG_INFO("Build Topology");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
    //
    // Now fill out the topology by creating the net devices required to connect
    // the nodes to the channels and hooking them up.
    //
    NetDeviceContainer devices = csma.Install(nodes);

    InternetStackHelper internet;
    internet.Install(nodes);

    // We've got the "hardware" in place.  Now we need to add IP addresses.
    //
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    //
    // Create an OnOff application to send UDP datagrams from node zero to node 1.
    //
    NS_LOG_INFO("Create Applications.");
    uint16_t port = 9; // Discard port (RFC 863)

    OnOffHelper onoff(cmd_socketFactory,
                      Address(InetSocketAddress(interfaces.GetAddress(1), port)));
    onoff.SetConstantRate(DataRate("500kb/s"));

    ApplicationContainer app = onoff.Install(nodes.Get(0));
    // Start the application
    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));

    // Create an optional packet sink to receive these packets
    PacketSinkHelper sink(cmd_socketFactory,
                          Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    app = sink.Install(nodes.Get(1));
    app.Start(Seconds(0.0));

    //
    // Create a similar flow from n3 to n0, starting at time 1.1 seconds
    //
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
    //
    // Configure ascii tracing of all enqueue, dequeue, and NetDevice receive
    // events on all devices.  Trace output will be sent to the file
    // "csma-one-subnet.tr"
    //
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll(ascii.CreateFileStream("csma-one-subnet.tr"));

    //
    // Also configure some tcpdump traces; each interface will be traced.
    // The output files will be named:
    //
    //     csma-one-subnet-<node ID>-<device's interface index>.pcap
    //
    // and can be read by the "tcpdump -r" command (use "-tt" option to
    // display timestamps correctly)
    //
    csma.EnablePcapAll("csma-one-subnet", false);
    //
    // Now, do the actual simulation.
    //

    Simulator::Run();
    Simulator::Stop(Seconds(30.0));
    Simulator::Destroy();

}
