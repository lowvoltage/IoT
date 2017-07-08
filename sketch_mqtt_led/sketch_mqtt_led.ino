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

// Use the ESP8266's MAC address to build unique topic names
char mqtt_in_topic[40];
char mqtt_out_topic[40];

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

  setup_wifi();

  // Once WiFi connection is established, build the unique topics names
  snprintf(mqtt_in_topic, 40, "device/%s/control", WiFi.BSSIDstr().c_str());
  snprintf(mqtt_out_topic, 40, "device/%s/status", WiFi.BSSIDstr().c_str());

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
  Serial.print(mqtt_out_topic);
  Serial.print("] ");
  Serial.println(message);

  // Flag all LED status messages as "Retained"
  mqttClient.publish(mqtt_out_topic, message, true);
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
    if (mqttClient.connect("ESP8266Client", MQTT_USERNAME, MQTT_PASSWORD,
                           mqtt_out_topic, 0, true, "N/A"))
    {
      Serial.println("connected");

      // ... and resubscribe
      mqttClient.subscribe(mqtt_in_topic);
      Serial.print("MQTT: Subscribed to ");
      Serial.println(mqtt_in_topic);
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
