#if !defined(ARDUINO_ARCH_ESP32)
#error "Select Tools > Board > ESP32 Arduino > ESP32 Dev Module"
#endif

#include <WiFi.h>
#include <WiFiUdp.h>

// ====== CONFIG ======
const char* AP_SSID = "GloveAP";
const char* AP_PASS = "glove1234";
const int   UDP_PORT = 5005;

// Pick your pin here:
const int CONTROL_PIN = 25;   // <- change to 13 if you want GPIO13 instead

WiFiUDP Udp;
char rxbuf[64];

void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, LOW);

  // Start Access Point
  Serial.println("\nStarting ESP32 Access Point...");
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(AP_SSID, AP_PASS);
  if (!ok) {
    Serial.println("AP start FAILED");
  } else {
    Serial.printf("AP started: SSID=%s  PASS=%s  IP=%s\n",
                  AP_SSID, AP_PASS, WiFi.softAPIP().toString().c_str());
  }

  // Start UDP listener
  Udp.begin(UDP_PORT);
  Serial.printf("UDP listening on %d\n", UDP_PORT);
}

void sendAck(const char* msg) {
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.print(msg);
  Udp.endPacket();
}

void loop() {
  int n = Udp.parsePacket();
  if (n > 0) {
    int len = Udp.read(rxbuf, sizeof(rxbuf) - 1);
    if (len < 0) return;
    rxbuf[len] = 0;

    // Uppercase for easy matching
    for (int i = 0; rxbuf[i]; ++i) rxbuf[i] = toupper((unsigned char)rxbuf[i]);

    Serial.printf("RX [%s:%u] -> \"%s\"\n",
                  Udp.remoteIP().toString().c_str(), Udp.remotePort(), rxbuf);

    if (strcmp(rxbuf, "ON") == 0) {
      digitalWrite(CONTROL_PIN, HIGH);
      sendAck("ACK:ON");
    } else if (strcmp(rxbuf, "OFF") == 0) {
      digitalWrite(CONTROL_PIN, LOW);
      sendAck("ACK:OFF");
    } else if (strcmp(rxbuf, "TOGGLE") == 0) {
      digitalWrite(CONTROL_PIN, !digitalRead(CONTROL_PIN));
      sendAck("ACK:TOGGLE");
    } else if (strncmp(rxbuf, "PULSE:", 6) == 0) {
      int ms = atoi(rxbuf + 6);
      if (ms < 0) ms = 0;
      if (ms > 5000) ms = 5000; // cap at 5s
      digitalWrite(CONTROL_PIN, HIGH);
      delay(ms);
      digitalWrite(CONTROL_PIN, LOW);
      sendAck("ACK:PULSE");
    } else {
      sendAck("ERR:UNKNOWN_CMD");
    }
  }

  delay(2);
}
