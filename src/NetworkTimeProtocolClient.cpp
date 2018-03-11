#include "NetworkTimeProtocolClient.h"

const char* const NetworkTimeProtocolClient::_ntpServers[] = {"0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org", "3.pool.ntp.org", 0};

NetworkTimeProtocolClient::NetworkTimeProtocolClient(UDP* aUDPInstance) {
	_UDP = aUDPInstance;
}

NetworkTimeProtocolClient::~NetworkTimeProtocolClient() {
	_UDP = 0;
}

uint32_t NetworkTimeProtocolClient::getCurrentNTPTime() {
	uint8_t serverIndex = 0;
	uint32_t currentNTPTime = 0;
	IPAddress serverIP;

	if(_UDP->begin(_localPort))	{
		while (currentNTPTime == 0 && _ntpServers[serverIndex] != 0) { 	// Iterates over virtual clusters [0-3].pool.ntp.org until success or list exhausted
			WiFi.hostByName(_ntpServers[serverIndex], serverIP);		// Destination IP is used later to ensure that the NTP request and response match
			Serial.print("Sending a NTP request packet to time server: ");
			Serial.print(_ntpServers[serverIndex]);
			Serial.print(" at IP address: ");
			Serial.println(serverIP);
			this->sendRequestNTPPacket(serverIP);
			currentNTPTime = this->readResponseNTPPacket(serverIP);
			serverIndex++;
		}
		if(currentNTPTime == 0) {
			Serial.println("Failed at sending NTP request packet to [0-3].pool.ntp.org servers");
		}
		_UDP->stop();
	} else {
		Serial.println("Failed to obtain an available socket!");
	}

	return currentNTPTime;
}

uint32_t NetworkTimeProtocolClient::getCurrentUnixEpochTime() {
	uint32_t currentEpochTime = this->getCurrentNTPTime();
	if(currentEpochTime > 0) {
		currentEpochTime = currentEpochTime - 2208988800UL; // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
	}
	return currentEpochTime;
}

void NetworkTimeProtocolClient::sendRequestNTPPacket(IPAddress& serverIP) {
	memset(_packetBuffer, 0, NTP_PACKET_SIZE); // Erase the _packetBuffer with zeros
	_packetBuffer[0] = 0b11100011;	// Leap indicator = Clock is unsynchronized = 3 (2 bits), NTP version = 4 (3 bits), NTP packet mode = Client = 3 (3 bits)
	_UDP->beginPacket(serverIP, _serverPort);
	_UDP->write(_packetBuffer, NTP_PACKET_SIZE);
	_UDP->endPacket();
}

uint32_t NetworkTimeProtocolClient::readResponseNTPPacket(IPAddress& serverIP) {
	uint8_t timeoutIndex = 0;
	uint16_t packetBytes = 0;
	uint32_t currentNTPTime = 0;

	while (timeoutIndex++ < 100 && packetBytes == 0) { // Timeout after ~1 sec if no NTP response packet is received
		packetBytes = _UDP->parsePacket();
		if (packetBytes > 0 && _UDP->remoteIP() != serverIP) { // UDP hole punching technique used by NAT could potentially create a request/response mismatch
			_UDP->flush(); // A previous time server has responded belatedly. Flush the packet.
			packetBytes = 0;
			Serial.print("NTP request/response mismatch! Received UDP packet from ");
			Serial.print(_UDP->remoteIP());
			Serial.print(" instead of ");
			Serial.println(serverIP);
		}
		if (packetBytes == 0) {
			delay(10);
		}
	}

	if (packetBytes > 0) {
		if (_UDP->available() == 48) {
			memset(_packetBuffer, 0, NTP_PACKET_SIZE); 	// Erase the _packetBuffer with zeros
			_UDP->read(_packetBuffer, NTP_PACKET_SIZE); // Read the packet into the buffer
			if (_packetBuffer[1] > 0 and _packetBuffer[1] < 16) { // Discard if not from Primary or Secondary time servers
				currentNTPTime =	static_cast<uint32_t>(_packetBuffer[40]) << 24 |
									static_cast<uint32_t>(_packetBuffer[41]) << 16 |
									static_cast<uint32_t>(_packetBuffer[42]) << 8 |
									static_cast<uint32_t>(_packetBuffer[43]);
			}
			else {
				Serial.println("Received NTP packet discarded! Was not coming from Primary or Secondary time servers.");
			}
		}
		else {
			Serial.println("Invalid NTP packet size received!");
		}

		Serial.print("UDP packet of ");
		Serial.print(packetBytes);
		Serial.print(" bytes received from IP: ");
		Serial.println(_UDP->remoteIP());
	}
	else {
		Serial.println("Timeout, no NTP response received!");
	}

	return currentNTPTime;
}