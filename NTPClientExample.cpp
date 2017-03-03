#include <WiFi101.h>
#include <WiFiUDP.h>
#include <TimeLib.h>
#include "NetworkTimeProtocolClient.h"

const char ssid[] = "Your SSID";
const char pass[] = "Your Password";
WiFiUDP myWiFiUDP;
NetworkTimeProtocolClient myNTPClient(&myWiFiUDP);

void connectToWiFi();
String printDatetime(uint32_t timestamp);
void print2digits(String& timestamp, int number);

void setup() {
	Serial.begin(9600);
	while (!Serial) {
		delay(10);
	}
	connectToWiFi();
}

void loop() {
	Serial.println("Going to NTP Server!");
	Serial.print(printDatetime(myNTPClient.getCurrentEpochTime()));
	Serial.println(" GMT");
	delay(1000 * 15); // Wait 15 sec
}

void connectToWiFi() {
	uint8_t counter = 0;
	while (WiFi.status() != WL_CONNECTED) { //  Retry until connected.
		Serial.print("Connecting to Wifi network: ");
		Serial.println(ssid);
		WiFi.begin(ssid, pass);
		while (WiFi.status() != WL_CONNECTED && counter++ < 10) { // Wait ~10 seconds before timing out.
			delay(1000);
		}
		if (WiFi.status() != WL_CONNECTED) {
			counter = 0;
			Serial.println("Failed to connect!");
		}
	}
	Serial.print("Connected to Wifi network: ");
	Serial.println(ssid);
}

String printDatetime(uint32_t timestamp) {
	String printedTimestamp;
	print2digits(printedTimestamp, year(timestamp));
	printedTimestamp += "/";
	print2digits(printedTimestamp, month(timestamp));
	printedTimestamp += "/";
	print2digits(printedTimestamp, day(timestamp));
	printedTimestamp += " ";

	print2digits(printedTimestamp, hour(timestamp));
	printedTimestamp += ":";
	print2digits(printedTimestamp, minute(timestamp));
	printedTimestamp += ":";
	print2digits(printedTimestamp, second(timestamp));

	return printedTimestamp;
}

void print2digits(String& timestamp, int number) {
	if (number < 10) {
		timestamp += "0";
	}
	timestamp += number;
}
