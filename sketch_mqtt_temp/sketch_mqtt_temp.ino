/*
 Temperature monitoring with ESP8266 over MQTT

 The ESP8266 connects to a MQTT server and publishes temperature readings
 from a DS18B20 One-Wire temperature sensor to a "status" topic
*/

#include <OneWire.h>
#include <DallasTemperature.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Replace with your MQTT credentials (optional)
#define MQTT_USERNAME NULL
#define MQTT_PASSWORD NULL
#define MQTT_SERVER "test.mosquitto.org"

// We'll use the ESP8266's MAC address to build unique topic names
char mqttOutTopic[40];

// Data wire is plugged into pin 2 on the NodeMCU ESP8266 (marked as D4 on the PCB)
#define ONE_WIRE_PIN 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_PIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Initialize with some "out of range" value, so that first actual reading always gets published
float lastPublishedTemperature = -1000.0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup(void)
{
  // Setup the serial port
  delay(1000);
  Serial.begin(115200);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the DallasTemperature library
  sensors.begin();

  setupWiFi();

  // Once WiFi connection is established, build the unique topics names
  snprintf(mqttOutTopic, 40, "device/%s/status", WiFi.BSSIDstr().c_str());
  
  mqttClient.setServer(MQTT_SERVER, 1883);
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

void publish(const char *message)
{
  Serial.print("MQTT: Publish message [");
  Serial.print(mqttOutTopic);
  Serial.print("] ");
  Serial.println(message);

  // Flag all status messages as "Retained"
  mqttClient.publish(mqttOutTopic, message, true);
}

void loop(void)
{
  // Ensure MQTT connection
  reconnect();

  // Read any incoming messages
  mqttClient.loop();

  // Perform a single reading
  float tempCelsius = readTemperature();

  // Print it over Serial
  Serial.print("Temperature is: ");
  Serial.println(tempCelsius);

  // Publish the new reading, if different from the old one
  if (fabs(lastPublishedTemperature - tempCelsius) > 0.1)
  {
    publish(("T = " + String(tempCelsius)).c_str());
    lastPublishedTemperature = tempCelsius;
  }

  // A small delay
  delay(1000);
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
                           mqttOutTopic, 0, true, "N/A"))
    {
      Serial.println("connected");
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

float readTemperature()
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  sensors.requestTemperatures();

  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  return sensors.getTempCByIndex(0);
}
