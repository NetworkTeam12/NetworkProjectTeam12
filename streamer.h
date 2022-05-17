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
			
			

	};

}

#endif
