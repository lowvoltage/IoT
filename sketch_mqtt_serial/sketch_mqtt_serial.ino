/*
 Serial "echo" with ESP8266 over MQTT

 The ESP8266 connects to a MQTT server and:
  - subscribes to "in" topic, printing out to Serial all incoming MQTT messages
  - publishes any text received over Serial to "out" topic
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Replace with your MQTT credentials (optional)
#define MQTT_USERNAME NULL
#define MQTT_PASSWORD NULL
#define MQTT_SERVER "test.mosquitto.org"

// We'll use the ESP8266's MAC address to build unique topic names
char mqttInTopic[40];
char mqttOutTopic[40];

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const unsigned int bufferSize = 50;
char outputBuffer[bufferSize];

void setup()
{
  delay(1000);
  Serial.begin(115200);

  setup_wifi();

  // Once WiFi connection is established, build the unique topics names
  snprintf(mqttInTopic, 40, "device/%s/in", WiFi.BSSIDstr().c_str());
  snprintf(mqttOutTopic, 40, "device/%s/out", WiFi.BSSIDstr().c_str());

  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(callback);
}

void setup_wifi()
{
  // WiFiManager: Connect with stored credentials or setup a Config AP
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP8266_Config_AP");

  Serial.println("");
  Serial.print("WiFi: Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("WiFi: IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("MQTT: Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void publish(const char *message)
{
  Serial.print("MQTT: Publish message [");
  Serial.print(mqttOutTopic);
  Serial.print("] ");
  Serial.println(message);

  mqttClient.publish(mqttOutTopic, message);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("MQTT: Attempting connection to ");
    Serial.print(MQTT_SERVER);
    Serial.print(" ...");

    // Attempt to connect
    if (mqttClient.connect("ESP8266Client", MQTT_USERNAME, MQTT_PASSWORD))
    {
      Serial.println("connected");

      // ... and resubscribe
      mqttClient.subscribe(mqttInTopic);
      Serial.print("MQTT: Subscribed to ");
      Serial.println(mqttInTopic);
    }
    else
    {

      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 30 seconds");
      delay(30000);
    }
  }
}

void loop()
{
  // Ensure MQTT connection
  reconnect();

  // Read any incoming messages
  mqttClient.loop();

  // Check the Serial input for data and publish it
  if (Serial.available() > 0)
  {
    // Leave a byte for the "manual" NULL-termination
    int n = Serial.readBytesUntil('\n', outputBuffer, bufferSize - 1);
    outputBuffer[n] = 0;

    publish(outputBuffer);
  }

  // Wait for a while
  delay(100);
}
