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
	.AddAttribute ("PacketNIP", 
                   "Number of packets in Frame",
                   UintegerValue(100),
                   MakeUintegerAccessor (&Client::m_packetNIP),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&Client::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&Client::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")

	;
	return tid;
}

Frame::Frame ()
{
	for (uint32_t i=0; i<1000; i++){
		m_packets[i] = 0;
		m_send[i] = 0;
	}
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
	m_sendN = 0;
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

	NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << "Client StartApplication" );
	m_socket->SetRecvCallback (MakeCallback (&Client::HandleRead, this));
	// m_consumEvent = Simulator::Schedule ( Seconds ((double)1.5), &Client::FrameConsumer, this);

	
}

void
Client::StopApplication ()
{
	if (m_socket != 0) 
	{
		m_socket->Close ();
		m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	}

	// Simulator::Cancel (m_consumEvent);
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
		m_frameN = seqN/m_packetNIP;
		NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " Client received packet from " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (from).GetPort () << " frame : " << m_frameN << " seq : " << seqN );
		PutFrameBuffer(m_frameN, seqN, from, socket);
	}
}

void 
Client::PutFrameBuffer (uint32_t frameN, uint32_t seqN, Address from, Ptr<Socket> socket)
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
	if (frameN > m_sendN){
		SendCheck(from, socket);
	}
	if (frameN > m_consumeN){
		FrameConsumer();
	}
}


void 
Client::SendCheck(Address from, Ptr<Socket> socket)
{
	NS_LOG_INFO("Resend frame : " << m_sendN);
	Frame frame = m_frameBuffer[m_sendN];
	uint32_t count = 0;
	for (uint32_t i=0; i<m_packetNIP; i++){
		if(frame.m_packets[i] == 0){
			Ptr<Packet> p = Create<Packet> (m_packetSize);
			SeqTsHeader header;
			header.SetSeq(m_packetNIP*m_sendN + i);
			p->AddHeader(header);
			socket->SendTo(p,0,from);
			NS_LOG_INFO("Requested Packet  frame :"<< m_sendN << " seq : " << i );
			m_frameBuffer[m_sendN].m_packets[i] = 2;
			m_frameBuffer[m_sendN].m_send[count] = i;
			count ++;
		}
	}
	m_sendN++;
}


void
Client::FrameConsumer ()
{
	Frame frame = m_frameBuffer[m_consumeN];

	bool full = true;
	for (uint32_t i=0; i<m_packetNIP; i++){
		if(frame.m_packets[frame.m_send[i]] == 2){
			full = false;
			break;
		}
		else if (frame.m_send[i] == 1){
			break;
		}
	}
	if(full){
		NS_LOG_INFO("Frame Consume" << m_consumeN);
		m_frameBuffer.erase(m_consumeN);
		m_consumeN++;
	}	
}


}
