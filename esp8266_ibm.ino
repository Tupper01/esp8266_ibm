//
// Jakob Schuldt Jensen DTU
//
#include <ArduinoJson.h>

#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7

//-------- Define Pins on the ESP -----------
const int tempPin = 0;
const int ledPin = 4;

//-------- Customise these values -----------
const char* ssid = "IBMFM";
const char* password = "Fiskemand";

#define ORG "xqnek7"
#define DEVICE_TYPE "ESP8266"
#define DEVICE_ID "myESP"
#define TOKEN "2_-t0l1JWcwQPGfI)"

//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char commandTopic[] = "iot-2/cmd/blink/fmt/json";
const char publishTopic[] = "iot-2/evt/status/fmt/json";
const char publishTemp[] = "iot-2/evt/temp/fmt/json";

//---------- Initialize all functions --------
void setup();
void loop();
void handleCommand(JsonObject& root);
void callback(char* topic, byte* payload, unsigned int payloadLength);
void publishTo(const char* topic, const char* payload);
void subscribeTo(const char* topic);
void wifiConnect();
void mqttConnect();

WiFiClient wifiClient; //Create wifi client object
PubSubClient client(server, 1883, callback, wifiClient); //Create a PubSub MQTT Client

int publishInterval = 5000; //5 seconds interval between status updates to status topic
long lastPublishMillis; //Interval counter

//---------- Setup the ESP, initialize wifi and mqtt connection

void setup() {
	pinMode(ledPin, OUTPUT);
	pinMode(tempPin, INPUT);

	Serial.begin(74880); Serial.println(); //Initialize Serial Monitor for verbose trouble shooting

	wifiConnect(); //Establish wifi-connection
	mqttConnect(); //Connect to MQTT-Broker once wifi is up
	subscribeTo(commandTopic); //Subscribe to the command-topic once connected to the MQTT Broker
}

//---------- The loop that does stuff, this is the center of the app

void loop() {
	if (millis() - lastPublishMillis > publishInterval) { //Check if interval and publish to status topic
		String payload = "{\"d\":{\"temp\":";
		payload += analogRead(tempPin);
		payload += "}}";
		publishTo(publishTopic, payload.c_str());
		lastPublishMillis = millis();
	}

	if (!client.loop()) { //Make sure we are connected to the MQTT Broker
		mqttConnect();
	}
}

void handleCommand(JsonObject& root) {
	// you will have to write your code here. The code that makes awesome stuff from the Cloud commands your send your device.


}

//---------- The callback is envoked when a MQTT message is sent to the device

void callback(char* topic, byte* payload, unsigned int payloadLength) { //Handle json callback from a topic we subscribe to
	Serial.print("callback invoked for topic: "); Serial.println(topic);

	StaticJsonBuffer<300> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject((char*)payload, payloadLength);
	if (!root.success()) {
		Serial.print("callback: payload parse FAILED: ");
		Serial.print(payloadLength); Serial.print(":"); Serial.print((char*)payload);
		return;
	}
	Serial.print("callback: payload parse OK: "); root.printTo(Serial); Serial.println();

	if (strcmp(commandTopic, topic) == 0) {
		handleCommand(root);
	}
	else {
		Serial.print("unexpected callback: ");
		Serial.println(topic);
	}
}

void publishTo(const char* topic, const char* payload) { //Publish to a specific MQTT topic
	Serial.print("publish ");
	if (client.publish(topic, payload)) {
		Serial.print(" OK ");
	}
	else {
		Serial.print(" FAILED ");
	}
	Serial.print(" topic: "); Serial.print(topic);
	Serial.print(" payload: "); Serial.println(payload);
}

void subscribeTo(const char* topic) { //Subscribe to a specific MQTT topic on the Broker
	Serial.print("subscribe ");
	if (client.subscribe(topic)) {
		Serial.print(" OK ");
	}
	else {
		Serial.print(" FAILED ");
	}
	Serial.print(" to: "); Serial.println(topic);
}

void wifiConnect() { //Connect to wifi
	Serial.print("Connecting to "); Serial.print(ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() { //Connect to MQTT Broker
	if (!!!client.connected()) {
		Serial.print("Reconnecting MQTT client to "); Serial.println(server);
		while (!!!client.connect(clientId, authMethod, token)) {
			Serial.print(".");
			delay(500);
		}
		Serial.println();
	}
}
noJsonJson