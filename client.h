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

	};


}

#endif
