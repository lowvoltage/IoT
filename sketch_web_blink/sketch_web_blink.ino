#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

ESP8266WebServer server(80);

String htmlHeader = "<h1>ESP8266 Web Server</h1>";
String htmlLedOn = htmlHeader + "<p>LED is ON <a href=\"off\"><button>Turn OFF</button></a></p>";
String htmlLedOff = htmlHeader + "<p>LED is OFF <a href=\"on\"><button>Turn ON</button></a></p>";

int ledState = LOW;

void setup(void){
  // Set GPIO to OUTPUT. Init the LED state
  pinMode(LED_BUILTIN, OUTPUT);
  setLed(LOW);

  delay(1000);
  Serial.begin(115200);

  setup_wifi();

  // Define server URLs
  server.on("/", [](){
    server.send(200, "text/html", ledState == HIGH ? htmlLedOn : htmlLedOff);
  });

  server.on("/on", [](){
    setLed(HIGH);
    redirectHome();
  });
  server.on("/off", [](){
    setLed(LOW);
    redirectHome();
  });

  server.begin();
  Serial.println("HTTP server started");
}

void setup_wifi() {
  // WiFiManager: Connect with stored credentials or setup a Config AP
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP8266_Config_AP");

  Serial.println("");
  Serial.print("WiFi: Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("WiFi: IP address: ");
  Serial.println(WiFi.localIP());
}

void setLed(int state) {
    // Store & set the LED state (LED_BUILTIN is inverted)
    ledState = state;
    digitalWrite(LED_BUILTIN, !ledState);
}

void redirectHome() {
    // HTTP redirect to the home page
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");
}
 
void loop(void) {
  server.handleClient();
} 
