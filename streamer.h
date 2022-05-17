#ifndef STREAMING_STREAMER_H
#define STREAMING_STREAMER_h

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

	class Socket;
	class Packet;

	class Streamer : public Application
	{
		public:
			static TypeId GetTypeId (void);
			Streamer ();
			virtual ~Streamer ();

		private:
			virtual void StartApplication (void);
			virtual void StopApplication (void);

			void SendPacket (void);
			void ReSendPacket (uint32_t seqN);
			void HandleRead (Ptr<Socket> socket);
			
			// Attribute 
			Address m_peerAddress;	// Destination address
			uint16_t m_peerPort;	// Destination port
			uint32_t m_packetSize 	// Packet size
			uint32_t m_fps;			// Streaming FPS
			bool m_lossEnable;		// Loss Enable 
			double m_lossRate;		// Loss Rate
			uint32_t m_packetNIP; 	// Number of packets in Frame

			// Additional member variables
			Ptr<Socket> m_socket;	// My socket
			EventId m_sendEvent;	// Send event
			uint32_t m_frameN; 	// Frame number attached to the packet header
			uint32_t m_seqN;	// Sequence number attached to the packet header


	};

}

#endif
