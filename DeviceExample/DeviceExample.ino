#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// Define the RX and TX pins for the GPS module
static const int RXPin = 17, TXPin = 16;
static const uint32_t GPSBaud = 115200;

// The TinyGPSPlus object
TinyGPSPlus gps;

// Use HardwareSerial instead of SoftwareSerial
HardwareSerial gpsSerial(1); // Use Serial1 (ESP32 supports multiple serial ports)

unsigned long lastUpdateTime = 0;  // Store the last time GPS data was printed

void setup()
{
  Serial.begin(115200);  // Default USB Serial for debugging
  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin); // Start GPS on Serial1

  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
  Serial.print(F("Testing TinyGPSPlus library v. ")); 
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

void loop()
{
  // Read data from GPS
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    Serial.write(c); // Print raw GPS data for debugging
    gps.encode(c);
  }

  // Print GPS data every 5 seconds
  if (millis() - lastUpdateTime >= 5000) {
    displayInfo();
    lastUpdateTime = millis(); // Update last print time
  }
}

void displayInfo()
{
  Serial.print(F("Местоположение: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("Няма валидни географски координати"));
  }

  Serial.print(F("  Дата/Час: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("Няма дата"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("Няма време"));
  }

  Serial.println();
}
