/*
 NodeMCU ESP8266 reading a DS18B20 One-Wire temperature sensor
 and publishing the results as a web page
*/
#include <OneWire.h>
#include <DallasTemperature.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

ESP8266WebServer webServer(80);

// The additional "meta" tag allows for the page to auto-refresh every 10 sec
String htmlHeader = 
  "<h1>ESP8266 Web Server</h1>"
  "<meta http-equiv=\"refresh\" content=\"10\" />"
  "<p>Temperature is ";
String htmlFooter = " degrees Celsius </p>";

// Data wire is plugged into pin 2 on the NodeMCU ESP8266 (marked as D4 on the PCB)
#define ONE_WIRE_PIN 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_PIN);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// The temperature reading variable needs to be global
float tempCelsius = 0.0;

void setup(void) {
  // Setup the serial port
  delay(1000);
  Serial.begin(115200);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the DallasTemperature library
  sensors.begin();

  setupWiFi();
  setupWebServer();
}

void setupWiFi() {
  // WiFiManager: Connect with stored credentials or setup a Config AP
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP8266_Config_AP");

  Serial.println("");
  Serial.print("WiFi: Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("WiFi: IP address: ");
  Serial.println(WiFi.localIP());
}

void setupWebServer() {
  // Define server URLs
  webServer.on("/", [](){
    // Convert the current temperature reading to a string and build the page HTML
    webServer.send(200, "text/html", htmlHeader + String(tempCelsius) + htmlFooter);
  });

  webServer.begin();
  Serial.println("HTTP server started");
}

void loop(void) { 
  // Perform a single reading
  tempCelsius = readTemperature();
  
  // Print it over Serial
  Serial.print("Temperature is: ");
  Serial.println(tempCelsius);

  // Handle incoming HTTP requests
  webServer.handleClient();
  
  // A small delay
  delay(100);
}

float readTemperature() {
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  sensors.requestTemperatures();
  
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  return sensors.getTempCByIndex(0);
}

