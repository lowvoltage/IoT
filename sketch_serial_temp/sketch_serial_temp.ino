/*
 NodeMCU ESP8266 reading a DS18B20 One-Wire temperature sensor
 and printing the results over Serial
*/
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the NodeMCU ESP8266 (marked as D4 on the PCB)
#define ONE_WIRE_PIN 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_PIN);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup(void) {
  // Setup the serial port
  delay(1000);
  Serial.begin(115200);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the DallasTemperature library
  sensors.begin();
}

void loop(void) {
  // Perform a single reading
  float tempCelsius = readTemperature();
  
  // Print it over Serial
  Serial.print("Temperature is: ");
  Serial.println(tempCelsius);

  // A small delay
  delay(1000);
}

float readTemperature() {
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  sensors.requestTemperatures();
  
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  return sensors.getTempCByIndex(0);
}

