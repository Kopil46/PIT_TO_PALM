#if !defined(ARDUINO_ARCH_ESP32)
#error "Select Tools > Board > ESP32 Arduino > ESP32 Dev Module"
#endif

#include <WiFi.h>

const char* AP_SSID = "ESP32_Test";      // network name
const char* AP_PASS = "12345678";        // must be at least 8 characters

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nStarting ESP32 Wi-Fi Access Point...");

  WiFi.mode(WIFI_AP);
  bool result = WiFi.softAP(AP_SSID, AP_PASS);

  if (result) {
    Serial.println("✅ Access Point started successfully!");
  } else {
    Serial.println("❌ Failed to start Access Point!");
  }

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.printf("SSID: %s\nPassword: %s\n", AP_SSID, AP_PASS);
}

void loop() {
  // show how many clients are connected
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 3000) {
    lastPrint = millis();
    Serial.printf("Connected stations: %d\n", WiFi.softAPgetStationNum());
  }
  delay(100);
}
