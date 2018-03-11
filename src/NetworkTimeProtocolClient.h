#ifndef NETWORKTIMEPROTOCOLCLIENT_H_
#define NETWORKTIMEPROTOCOLCLIENT_H_

#include <Arduino.h>
#include <Udp.h>
#include <WiFi101.h>

#define NTP_PACKET_SIZE 48 	// NTP time stamp is in the first 48 bytes of the message

class NetworkTimeProtocolClient {
private:
	UDP* _UDP;
	static const uint8_t _serverPort = 123;		// Remote port to send NTP Request packet
	static const uint16_t _localPort = 2390;	// Local port to listen for NTP Response packet
	static const char* const _ntpServers[]; 	// Reliable virtual clusters providing time servers geographically close via round robin DNS
	uint8_t _packetBuffer[NTP_PACKET_SIZE];		// Buffer to hold incoming and outgoing packets
	void sendRequestNTPPacket(IPAddress& serverIP);
	uint32_t readResponseNTPPacket(IPAddress& serverIP);

public:
	NetworkTimeProtocolClient(UDP* aUDPInstance);
	~NetworkTimeProtocolClient();
	uint32_t getCurrentNTPTime();		// NTP uses an epoch of January 1, 1900 
	uint32_t getCurrentUnixEpochTime(); // Unix uses an epoch of January 1, 1970 
};

#endif /* NETWORKTIMEPROTOCOLCLIENT_H_ */
