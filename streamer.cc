#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"
#include "ns3/double.h"
#include "ns3/boolean.h"

#include "streaming-streamer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StreamingStreamerApplication");

NS_OBJECT_ENSURE_REGISTERED (Streamer);

TypeId
Streamer::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::Streamer")
		.SetParent<Application> ()
		.AddConstructor<Streamer> ()
    .AddAttribute ("Remote", 
                   "Address of destination to be streamed",
                   AddressValue (),
                   MakeAddressAccessor (&Streamer::m_peerAddress),
                   MakeAddressChecker ())
		.AddAttribute ("Port", 
                   "Port of destination to be streamed",
                   UintegerValue (0),
                   MakeUintegerAccessor (&Streamer::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
		.AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&Streamer::m_packetSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("StreamingFPS", 
                   "Streaming FPS",
                   UintegerValue (90),
                   MakeUintegerAccessor (&Streamer::m_fps),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("LossEnable", 
                   "Forced Packet Loss on/off",
                   BooleanValue (false),
                   MakeBooleanAccessor (&Streamer::m_lossEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("LossRate", 
                   "Loss probability",
                   DoubleValue (0.01),
                   MakeDoubleAccessor (&Streamer::m_lossRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PacketNIP", 
                   "Number of packets in Frame",
                   UintegerValue(100),
                   MakeUintegerAccessor (&StreamingStreamer::m_packetNIP),
                   MakeUintegerChecker<uint32_t> ())
	;
	return tid;
}

Streamer::Streamer ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_sendEvent = EventId ();
  m_frameN = 0;
	m_seqN = 0;
}

Streamer::~Streamer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
}

void 
Streamer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (!m_socket)
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);

    if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
    {
      if (m_socket->Bind () == -1)
      {
        NS_FATAL_ERROR ("Failed to bind socket");
      }
      m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    }
    else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
    {
      if (m_socket->Bind () == -1)
      {
        NS_FATAL_ERROR ("Failed to bind socket");
      }
      m_socket->Connect (m_peerAddress);
    }
    else
    {
      NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
    }
  }

  m_socket->SetRecvCallback (MakeCallback (&Streamer::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  Send();
}

void 
Streamer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
  {
    m_socket->Close ();
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    m_socket = 0;
  }
  Simulator::Cancel (m_sendEvent);
}


void 
Streamer::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());

	for (uint32_t i=0; i<m_packetNIP; i++){ 
    if (m_lossEnable){
			double rand = rand() % 100; // random for 0-99
			if (rand < m_lossRate) continue;
		}

		Ptr<Packet> p = Create<Packet> (m_packetSize);

		SeqTsHeader seqTs;	
		seqTs.SetSeq (m_seqN++);
		p->AddHeader (seqTs);
		m_socket->Send (p);
      
    Address localAddress;
	  m_socket->GetSockName (localAddress);

	}
  
	m_frameN += 1;
  m_sendEvent = Simulator::Schedule ( Seconds ((double)1.0/m_fps), &Streamer::Send, this);
}

void
Streamer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  
	while ((packet = socket->RecvFrom (from)))
  {
    if (InetSocketAddress::IsMatchingType (from))
    {
			
			SeqTsHeader seqTs;
			packet->RemoveHeader (seqTs);
			uint32_t seqN = seqTs.GetSeq();
			ReSendPacket(seqN);
    }
    socket->GetSockName (localAddress);
	}
}


}