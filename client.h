#ifndef STREAMING_CLIENT_H
#define STREAMING_CLIENT_h

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"

#include <map>



namespace ns3{

	class Socket;
	class Packet;

	class Frame
	{
		public:
			Frame ();
			~Frame ();
			int m_packets[1000];
	};


	class Client : public Application
	{
		public:
			static TypeId GetTypeId (void);
			Client ();
			virtual ~Client ();

			void PutFrameBuffer (uint32_t frameN, uint32_t seqN);
			void FrameConsumer (void);

		private:
			virtual void StartApplication (void);
			virtual void StopApplication(void);

			void HandleRead (Ptr<Socket> socket);
			

			// Attribute 
			Address m_peerAddress;	// Destination address
			uint16_t m_peerPort;	// Destination port
			uint32_t m_packetSize 	// Packet size
			uint32_t m_bufferSize; 	// Frame buffer size
			bool m_lossEnable;		// Loss Enable 
			double m_lossRate;		// Loss Rate
			uint32_t m_packetNIP; 	// Number of packets in Frame
			uint32_t m_resume;		
			uint32_t m_pause;

			
			

	};


}

#endif
