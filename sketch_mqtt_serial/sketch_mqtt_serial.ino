/*
 Basic ESP8266 MQTT example, a variation of "mqtt_esp8266" example that comes with 
 the PubSubClient library

 It connects to a MQTT server and:
  - subscribes to "inTopic", printing out any messages it receives
  - publishes any text received over Serial to "outTopic"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Replace with your network credentials
#define WIFI_SSID "your-network-ssid"
#define WIFI_PASSWORD "your-network-password"

// Replace with your MQTT credentials (optional)
#define MQTT_USERNAME NULL
#define MQTT_PASSWORD NULL
#define MQTT_SERVER "test.mosquitto.org"

#define MQTT_IN_TOPIC "inTopic"
#define MQTT_OUT_TOPIC "outTopic";

// Overwrite default credentials with local ones
#include "credentials.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const unsigned int bufferSize = 50;
char outputBuffer[bufferSize];

void setup() {
  delay(1000);
  Serial.begin(115200);

  setup_wifi();
  
  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(callback);
}

void setup_wifi() {
  WiFi.mode(WIFI_OFF);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi: Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("WiFi: IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT: Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void publish(const char* message) {
  Serial.print("MQTT: Publish message [");
  Serial.print(MQTT_OUT_TOPIC);
  Serial.print("] ");
  Serial.println(message);

  mqttClient.publish(MQTT_OUT_TOPIC, message);
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("MQTT: Attempting connection to ");
    Serial.print(MQTT_SERVER);
    Serial.print(" ...");
    
    // Attempt to connect
    if (mqttClient.connect("ESP8266Client", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
      
      // ... and resubscribe
      mqttClient.subscribe(MQTT_IN_TOPIC);
      Serial.print("MQTT: Subscribed to ");
      Serial.println(MQTT_IN_TOPIC);

    } else {
      
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 30 seconds");
      delay(30000);
    }
  }
}

void loop() {
  // Ensure MQTT connection
  reconnect();

  // Read any incoming messages
  mqttClient.loop();

  // Check the Serial input for data and publish it
  if (Serial.available() > 0) {
    // Leave a byte for the "manual" NULL-termination
    int n = Serial.readBytesUntil('\n', outputBuffer, bufferSize - 1);
    outputBuffer[n] = 0;
    
    publish(outputBuffer);
  } 

  // Wait for a while
  delay(100);
}

