/*
 LED-control with ESP8266 over MQTT

 The ESP8266 connects to a MQTT server and:
  - subscribes to a dedicated "control" topic and responds to messages by 
    switching the LED on and off
  - publishes the current LED state on a dedicated "state" topic
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

// We'll use the ESP8266's MAC address to build unique ID and topic names
char mqttID[20];
char mqttInTopic[40];
char mqttOutTopic[40];

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const unsigned int bufferSize = 50;
char inputBuffer[bufferSize];

int publishedLedState = -1;

void setup()
{
  // Setup the LED pin. LED if off on startup
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

  delay(1000);
  Serial.begin(115200);

  setupWiFi();

  // Once WiFi connection is established, build the unique topics names
  strcpy(mqttID, WiFi.BSSIDstr().c_str());
  snprintf(mqttInTopic, 40, "device/%s/control", mqttID);
  snprintf(mqttOutTopic, 40, "device/%s/status", mqttID);

  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(callback);
}

void setupWiFi()
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
  // Copy the payload in a buffer and NULL-terminate it
  int minLength = _min(length, bufferSize - 1);
  strncpy(inputBuffer, (char *)payload, minLength);
  inputBuffer[minLength] = '\0';

  Serial.print("MQTT: Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(inputBuffer);

  // Two messages will switch the LED on: "ON" and "1"
  // For BUILTIN_LED, the actual pin-levels are inverted
  if (0 == strcmp(inputBuffer, "ON") || 0 == strcmp(inputBuffer, "1"))
  {
    digitalWrite(BUILTIN_LED, LOW);
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

void publish(const char *message)
{
  Serial.print("MQTT: Publish message [");
  Serial.print(mqttOutTopic);
  Serial.print("] ");
  Serial.println(message);

  // Flag all LED status messages as "Retained"
  mqttClient.publish(mqttOutTopic, message, true);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("MQTT: Attempting connection to ");
    Serial.print(MQTT_SERVER);
    Serial.print(" ...");

    // Attempt to connect. Set a "N/A" message as a MQTT "Last Will"
    if (mqttClient.connect(mqttID, MQTT_USERNAME, MQTT_PASSWORD,
                           mqttOutTopic, 0, true, "N/A"))
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

  // Keep the published LED state up-to-date
  int currentLedState = digitalRead(BUILTIN_LED);
  if (publishedLedState != currentLedState)
  {
    // For BUILTIN_LED, the actual pin-levels are inverted
    publish(currentLedState == HIGH ? "OFF" : "ON");
    publishedLedState = currentLedState;
  }

  // Wait for a while
  delay(100);
}
