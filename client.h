#ifndef STREAMING_CLIENT_H
#define STREAMING_CLIENT_h

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
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
			int m_send[1000];
	};


	class Client : public Application
	{
		public:
			static TypeId GetTypeId (void);
			Client ();
			virtual ~Client ();

			void PutFrameBuffer (uint32_t frameN, uint32_t seqN, Address from, Ptr<Socket> socket);
			void FrameConsumer ();
			void SendCheck(Address from, Ptr<Socket> socket);

		private:
			virtual void StartApplication (void);
			virtual void StopApplication(void);

			void HandleRead (Ptr<Socket> socket);
			

			// Attribute 
			Address m_peerAddress;	// Destination address
			uint16_t m_peerPort;	// Destination port
			uint32_t m_packetSize;	// Packet size
			uint32_t m_bufferSize; 	// Frame buffer size
			uint32_t m_packetNIP; 	// Number of packets in Frame
			uint32_t m_resume;		
			uint32_t m_pause;
			TracedCallback<Ptr<const Packet> > m_rxTrace;
			TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;

			// Additional member variables
			Ptr<Socket> m_socket;	// My socket
			Address m_local;		// Local adderss
			EventId m_consumEvent;  // Consume event
			uint32_t m_frameN; 	// Frame number attached to the packet header
			uint32_t m_seqN;	// Sequence number attached to the packet header
			std::map<uint32_t, Frame> m_frameBuffer;	// Frame buffer -> (FrameIndex, Frame)
			uint32_t m_consumeN; // The index of the frame that should now be consumed
			uint32_t m_sendN;
			
			

	};


}

#endif
