#ifndef STREAMING_STREAMER_H
#define STREAMING_STREAMER_h

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include <vector>


namespace ns3 {

class Socket;
class Packet;

class DiscriminationStreaming : public Application
{
public:
	static TypeId GetTypeId (void);
	DiscriminationStreaming ();
	virtual ~DiscriminationStreaming ();


private:
	virtual void StartApplication (void);
	virtual void StopApplication (void);

	void HandleRead (Ptr<Socket> socket);
	void SendPacket (void);
	void ScheduleTx (Time dt);
	void addclient(Address addr); //to do
	void dropclient(Address addr); //to do


	uint32_t m_size;
	vector<Address> addrs;
	uint32_t m_sent;
	Ptr<Socket> m_socket;

};

}

#endif
