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

#include <cstdlib>
#include <ctime>
#include "streamer.h"

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
                   UintegerValue (30),
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
                   MakeUintegerAccessor (&Streamer::m_packetNIP),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Mode", 
                   "Select congestion control mode",
                   UintegerValue(0),
                   MakeUintegerAccessor (&Streamer::m_mode),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("threshold", 
                   "Select threshold",
                   UintegerValue(200),
                   MakeUintegerAccessor (&Streamer::m_threshold),
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
  m_slowstart = true;
  m_flag = 0;
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
  
  NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << "Streamer StartApplication" );
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
	    		srand(time(NULL));
			double randN =  (rand() % 10000) / 100; // random for 0-99
			// NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << "Streamer loss enable" );
			if (randN < m_lossRate * 100) {
        NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) <<" frame : " << m_frameN << " seq : "  << m_seqN << " packet loss" );
        m_seqN++;
        continue;
      }
		}

		Ptr<Packet> p = Create<Packet> (m_packetSize);

		SeqTsHeader seqTs;	
		seqTs.SetSeq (m_seqN);
		p->AddHeader (seqTs);
		m_socket->Send (p);
    
		NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " Streamer sent "  <<  Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort << " frame : " << m_frameN << " seq : "  << m_seqN );
    m_seqN++;
		Address localAddress;
	  m_socket->GetSockName (localAddress);

	}
	m_frameN += 1;
  Flowcontrol(0,1);
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
	    NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " Streamer received " << packet->GetSize () << " bytes from " <<  InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<  InetSocketAddress::ConvertFrom (from).GetPort ());
			SeqTsHeader seqTs;
			packet->RemoveHeader (seqTs);
			uint32_t seqN = seqTs.GetSeq();
			ReSend(seqN);

    }
    socket->GetSockName (localAddress);
	}
}

void 
Streamer::ReSend (uint32_t seqN)
{
  NS_LOG_FUNCTION (this);

	Ptr<Packet> p = Create<Packet> (m_packetSize);
  Flowcontrol(1, seqN); //congestion control
	SeqTsHeader seqTs;	
	seqTs.SetSeq (seqN);
	p->AddHeader (seqTs);
  m_socket->Send (p);
  NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " Streamer resent packet, seq : "  << seqN );
  Address localAddress;
	m_socket->GetSockName (localAddress);
}

void
Streamer::Flowcontrol(uint32_t drop, uint32_t seqN){
  NS_LOG_DEBUG ("1\t" << Simulator::Now().GetSeconds() << "\t" <<  m_fps );
  if(m_mode == 0){ // AIMD
    if(drop == 1){
      m_fps = m_fps / 2;
      if(m_fps < 30){
        m_fps = 30;
      }
    }
    else{
      m_fps++;
    }
    NS_LOG_INFO("Fps : " << m_fps);
  } 
  else if(m_mode == 1){ // Slow Start
    if(drop == 1){
      if(m_slowstart){
        if(m_seqN -3 > seqN){
          m_slowstart = false;
          m_threshold = m_fps / 2;
          m_fps = 30;
        }
        else{
          m_fps = 30;
        }
      }
      else{
        m_threshold = m_fps / 2;
        m_fps = 30;
      }
    }
    else{
      if(m_seqN < m_threshold){ //Slow start
          m_fps = m_fps * 2;
        }
      else{ //Congestion avoidance
        m_fps++;
      }
    }
  }
  else{ // Not used
  }
}
}
