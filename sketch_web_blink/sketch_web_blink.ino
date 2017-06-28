#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// Replace with your network credentials
const char* ssid = "your-network-ssid";
const char* password = "your-network-password";

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

  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF); 
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

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
