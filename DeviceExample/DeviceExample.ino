#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include "GxEPD2_display_selection_new_style.h"
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"

extern "C" {
  #include "qrcode.h"
}

char gnss_time[50] = "Urban GPS Tracker";
char gnss_lat[25] = "LAT: --.------";
char gnss_lng[25] = "LNG: --.------";
char qr_link[128] = "https://maps.google.com/?q=--.--,--.--";

static const int RXPin = 32, TXPin = 33;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

// Buttons
const int buttonRefresh = 26;
const int buttonQR = 27;
const int buttonToggleMode = 25;

bool showQR = false;
bool useHardcoded = false;

void setup() {
  Serial.begin(9600);
  ss.begin(GPSBaud);

  pinMode(buttonRefresh, INPUT_PULLUP);
  pinMode(buttonQR, INPUT_PULLUP);
  pinMode(buttonToggleMode, INPUT_PULLUP);

  display.init(9600, true, 2, false); 
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  PrintData();  // Initial display
}

void getInfo() {
  if (useHardcoded) {
    strcpy(gnss_lat, "LAT: 42.655536");
    strcpy(gnss_lng, "LNG: 23.289287");
    strcpy(qr_link, "https://maps.google.com/?q=42.655536,23.289287");
    strcpy(gnss_time, "Test Mode: Static Data");
    Serial.println("[Test Mode] Using hardcoded coordinates.");
  } else if (gps.location.isValid()) {
    snprintf(gnss_lat, sizeof(gnss_lat), "LAT: %.6f", gps.location.lat());
    snprintf(gnss_lng, sizeof(gnss_lng), "LNG: %.6f", gps.location.lng());
    snprintf(qr_link, sizeof(qr_link), "https://maps.google.com/?q=%.6f,%.6f",
             gps.location.lat(), gps.location.lng());

    if (gps.date.isValid() && gps.time.isValid()) {
      snprintf(gnss_time, sizeof(gnss_time), "%02d/%02d/%04d %02d:%02d:%02d UTC",
               gps.date.day(), gps.date.month(), gps.date.year(),
               gps.time.hour(), gps.time.minute(), gps.time.second());
    } else {
      strcpy(gnss_time, "Invalid Date/Time");
    }

    Serial.println("[Live GPS] Data updated:");
  } else {
    strcpy(gnss_lat, "LAT: N/A");
    strcpy(gnss_lng, "LNG: N/A");
    strcpy(qr_link, "");
    strcpy(gnss_time, "Waiting for GPS...");
    Serial.println("[Live GPS] No valid signal.");
  }

  Serial.println(gnss_lat);
  Serial.println(gnss_lng);
  Serial.println(gnss_time);
  Serial.println(qr_link);
}

void drawQRCode(const char* data) {
  QRCode qrcode;

  const int qrVersion = 4;
  const uint8_t eccLevel = 0; // ECC_LOW

  uint8_t qrcodeData[qrcode_getBufferSize(qrVersion)];
  qrcode_initText(&qrcode, qrcodeData, qrVersion, eccLevel, data);

  const int scale = 4;  // Bigger QR modules
  int qrPixelSize = qrcode.size * scale;

  // Centering the QR code
  int offset_x = (display.width() - qrPixelSize) / 2;
  int offset_y = (display.height() - qrPixelSize) / 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      int px = offset_x + x * scale;
      int py = offset_y + y * scale;

      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(px, py, scale, scale, GxEPD_BLACK);
      }
    }
  }
}

void PrintData() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    if (showQR) {
      if (strlen(qr_link) > 0) {
        drawQRCode(qr_link);
      } else {
        display.setCursor(10, 30);
        display.println("No satellite coverage");
      }
    } else {
      display.setCursor(10, 25);
      display.println(gnss_time);
      display.setCursor(10, 50);
      display.println(gnss_lat);
      display.setCursor(10, 75);
      display.println(gnss_lng);
    }
  } while (display.nextPage());
}

void loop() {
  while (ss.available()) {
    gps.encode(ss.read());
  }

  if (millis() > 5000 && gps.charsProcessed() < 10 && !useHardcoded) {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }

  // Button 1: Refresh
  if (digitalRead(buttonRefresh) == LOW) {
    getInfo();
    PrintData();
    delay(300);
  }

  // Button 2: Toggle QR Mode
  if (digitalRead(buttonQR) == LOW) {
    showQR = !showQR;
    PrintData();
    delay(300);
  }

  // Button 3: Toggle Test Mode (Hardcoded)
  if (digitalRead(buttonToggleMode) == LOW) {
    useHardcoded = !useHardcoded;
    Serial.print("Switched to ");
    Serial.println(useHardcoded ? "Test Mode (hardcoded data)" : "Live GPS Mode");
    getInfo();
    PrintData();
    delay(300);
  }
}
