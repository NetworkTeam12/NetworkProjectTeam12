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


}