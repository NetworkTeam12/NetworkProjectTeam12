#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"
#include "ns3/double.h"
#include "ns3/boolean.h"

#include "client.h"
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StreamingClientApplication");

NS_OBJECT_ENSURE_REGISTERED (Client);

TypeId
Client::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::Client")
	.SetParent<Application> ()
	.SetGroupName("Applications")
	.AddConstructor<Client> ()
	.AddAttribute ("Remote", 
                   "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&Client::m_peerAddress),
                   MakeAddressChecker ())
	.AddAttribute ("Port",
					"Port on which we listen for incoming packets.",
					UintegerValue (9),
					MakeUintegerAccessor (&Client::m_peerPort),
					 MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", 
                   "The size of packets receive in on state",
					UintegerValue (100),
					MakeUintegerAccessor (&Client::m_packetSize),
					MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("BufferSize", 
                   "The frame buffer size",
					UintegerValue (40),
					MakeUintegerAccessor (&Client::m_bufferSize),
					MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("LossEnable", 
                   "Whether to force loss or not to occur",
                   BooleanValue (false),
                   MakeBooleanAccessor (&Client::m_lossEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("LossRate", 
                   "Loss probability",
                   DoubleValue (0.01),
                   MakeDoubleAccessor (&Client::m_lossRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PauseSize", 
                   "The pause size for the frame buffer",
					UintegerValue (30),
					MakeUintegerAccessor (&Client::m_pause),
					MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ResumeSize", 
                   "The resume size for the frame buffer",
					UintegerValue (5),
					MakeUintegerAccessor (&Client::m_resume),
					MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("PacketNIP", 
                   "Number of packets in Frame",
                   UintegerValue(100),
                   MakeUintegerAccessor (&StreamingStreamer::m_packetNIP),
                   MakeUintegerChecker<uint32_t> ())

	;
	return tid;
}

Frame::Frame ()
{
	for (uint32_t i=0; i<m_packetNIP; i++)
		m_packets[i] = 0;
	m_consume = 0;
}

Frame::~Frame()
{

}


Client::Client ()
{
	NS_LOG_FUNCTION (this);
	m_consumEvent = EventId ();
  	m_frameN = 0;
	m_seqN = 0;
	m_consumeN = 0;
}

Client::~Client ()
{
	NS_LOG_FUNCTION (this);
	m_socket = 0;
}

void 
Client::StartApplication (void)
{
	NS_LOG_FUNCTION (this);
	
	if (m_socket == 0)
  	{
		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		m_socket = Socket::CreateSocket (GetNode (), tid);
		InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_peerPort);

		if (m_socket->Bind (local) == -1)
		{
			NS_FATAL_ERROR ("Failed to bind socket");
		}
    
		if (addressUtils::IsMulticast (m_local))
		{
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
			if (udpSocket)
			{
				// equivalent to setsockopt (MCAST_JOIN_GROUP)
				udpSocket->MulticastJoinGroup (0, m_local);
			}
			else
			{
				NS_FATAL_ERROR ("Error: Failed to join multicast group");
			}
		}
  	}
	if (m_packetNIP < 1 && m_packetNIP > 1000 ){
		NS_FATAL_ERROR ("Error: Too many packets in frame. Set the characteristics a little less.");
	}

	m_socket->SetRecvCallback (MakeCallback (&Client::HandleRead, this));
	m_consumEvent = Simulator::Schedule ( Seconds ((double)1.5), &Client::FrameConsumer, this);

}

void
Client::StopApplication ()
{
	if (m_socket != 0) 
	{
		m_socket->Close ();
		m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	}

	Simulator::Cancel (m_consumEvent);
}

void
Client::HandleRead (Ptr<Socket> socket)
{
	NS_LOG_FUNCTION (this << socket);

	Ptr<Packet> packet;
	Address from;
	Address localAddress;

	while ((packet = socket->RecvFrom (from)))
	{
		socket->GetSockName (localAddress);

		SeqTsHeader seqTs;
		packet->RemoveHeader (seqTs);
		
		uint32_t seqN = seqTs.GetSeq();
		uint32_t frameN = seqN/m_packetNIP;

		PutFrameBuffer(packetN, seqN)
	}
}

void 
Client::PutFrameBuffer (uint32_t frameN, uint32_t seqN)
{
	Frame frame;
	if ( m_frameBuffer.find(frameN) != m_frameBuffer.end()) {
		// Frame exists -> put the frame 
		frame = m_frameBuffer[frameN];
		frame.m_packets[seqN % m_packetNIP] = 1;
		m_frameBuffer[frameN] = frame;

	} else {
		// Frame does not exist
		if( m_frameBuffer.size() >= m_bufferSize ){
			// 1. buffer is full
			NS_LOG_INFO("Frame buffer is full.");
		}
		else { 
			// 2. buffer is not full 
			frame.m_packets[seqN % m_packetNIP] = 1;
			m_frameBuffer.insert(std::make_pair(frameN, frame));
		}
	}
}


void
Client::FrameConsumer (void)
{
	if( m_frameBuffer.size() > 0 ){
		if (m_frameBuffer.find(m_consumeN) != m_frameBuffer.end()) {
			// Frame exists -> check the packet -> request the pakcet

			int resend = 0;
			Frame frame = m_frameBuffer[m_consumeN];
			for (int i=0; i<m_packetNIP; i++){
				if(frame.m_packets[i] == 0){
					Ptr<Packet> p = Create<Packet> (m_packetSize);
					
					SeqTsHeader header;
					header.SetSeq(m_packetNIP*m_consumeN + i);
					p->AddHeader(header);

					Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
					udpSocket->SendTo (p, 0, m_peerAddress);

					resend = 1;
				}
			}

			if(resend==0){
				m_frameBuffer.erase(m_consumeN++);
			}
			
		} else {
			// Frame does not exist -> request the frame

			for (uint32_t i=0; i<m_packetNIP; i++){
				Ptr<Packet> p = Create<Packet> (m_packetSize);
					
				SeqTsHeader header;
				header.SetSeq(m_packetNIP*m_consumeN + i);
				p->AddHeader (header);

				Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
				udpSocket->SendTo (p, 0, m_peerAddress);
			}
			
		}
	}

	m_consumEvent = Simulator::Schedule ( Seconds ((double)1.0), &Client::FrameConsumer, this);
}


}
