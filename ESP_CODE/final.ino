#if !defined(ARDUINO_ARCH_ESP32)
#error "Select Tools > Board > ESP32 Arduino > ESP32 Dev Module"
#endif

#include <WiFi.h>
#include <WiFiUdp.h>

// ===== Wi-Fi AP =====
const char* AP_SSID = "GloveAP";
const char* AP_PASS = "glove1234";
const int   UDP_PORT = 5005;

// ===== PWM / IO =====
const int PWM_PIN  = 25;      // transistor base / LED
const int PWM_PINL = 26;
const int PWM_FREQ = 2000;    // 2 kHz
const int PWM_BITS = 8;
const int MAX_DUTY = 255;

// ===== Patterns (values 0..1) =====
// P1: your ramp/valley shape
const float pattern1[] = {
  0.0,0.0,0.0,0.0, 0.2,0.4,0.5,0.6,0.7,0.8,1,0.8,0.5,0.5,
  0.5,0.6,0.8,1,0.72,0.7,0.5,0.6,0.7,0.75,0.85,0.7,0.6, 0.0,0.0,0.0,0.0
};
//Pattern for the left glove
const float pattern1Left[] = {
  0.0,0.0,0.0,0.0, 0.2,0.4,0.6,0.8,0.0,0.0,0.0,0.0,0.0,0.6,
  0.5,0.0,0.0,0.0,0.5,0.6,0.7,0.2,0.1,0.1,0.1,0.1,0.6, 0.0,0.0,0.0,0.0
};
// P2: long on/off blocks (1s steps)
// const float pattern2[] = { 0,0,0, 1,1,1,1, 0,0,0,0 };


//*******************
const float pattern2[] = {0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
1,
0,
1,
0,
0,
0,
1,
0,
1,
0,
0,
0,
1,
0,
1,
0,
0,
0,
1,
0,
1,
0,
0,
0,
1,
0,
1,
0,
0,
0,
1,
0,
1,
0,
0,
0,
1,
0,
1,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0};


//*******************




// P3: your mixed small hits then 3 strong pops
const float pattern3[] = {
  0.0,0.15,0.3,0.0,0.3,0,0.3,0,0.3,0,0.3,0, 0.9,0,0.9,0,0.9,0,
  0,0,0,0,0,0,0,0,0,0,0,0
};

WiFiUDP Udp;
bool stopRequested = false;

// --- helpers ---
inline void writeDutyFrac(float f) {
  if (f < 0) f = 0; if (f > 1) f = 1;
  ledcWrite(PWM_PIN, int(f * MAX_DUTY));
}

// FOR THE LEFT GLOVE
inline void writeDutyFracL(float f) {
  if (f < 0) f = 0; if (f > 1) f = 1;
  ledcWrite(PWM_PINL, int(f * MAX_DUTY));
}

inline void allOff() { ledcWrite(PWM_PIN, 0); ledcWrite(PWM_PINL, 0); }

void sendAck(const char* msg) {
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.print(msg);
  Udp.endPacket();
}

bool checkStop() {
  // non-blocking peek for a STOP while a pattern is running
  int n = Udp.parsePacket();
  if (n > 0) {
    char b[32];
    int len = Udp.read(b, sizeof(b)-1);
    if (len > 0) {
      b[len] = 0;
      for (int i=0; b[i]; ++i) b[i] = toupper((unsigned char)b[i]);
      if (strcmp(b,"S")==0 || strcmp(b,"STOP")==0) {
        stopRequested = true;
        sendAck("ACK:STOP");
        return true;
      }
    }
  }
  return false;
}

// play a float pattern at fixed step_ms, can be interrupted by S/STOP
void playPattern(const float* pat, size_t len, int step_ms) {
  stopRequested = false;
  for (size_t i = 0; i < len; ++i) {
    if (stopRequested || checkStop()) break;
    writeDutyFrac(pat[i]);
    delay(step_ms);
  }
  allOff();
}

// play a float pattern for the defence video: contains pattern1 and pattern1Left
void playPatternRL(const float* pat,const float* patL, size_t len, int step_ms) {
  stopRequested = false;
  for (size_t i = 0; i < len; ++i) {
    if (stopRequested || checkStop()) break;
    writeDutyFrac(pat[i]);
    writeDutyFracL(patL[i]);
    delay(step_ms);
  }
  allOff();
}

// P4: “boost / nitro” — 6 fast buzzes in ~0.5 s total
void patternBoost() {
  stopRequested = false;
  const int bursts = 6;
  const int period_ms = 500 / bursts;   // ≈83 ms each
  const int on_ms = 40;                  // ~40 ms ON
  for (int i=0; i<bursts; ++i) {
    if (stopRequested || checkStop()) break;
    ledcWrite(PWM_PIN, 255);
    delay(on_ms);
    ledcWrite(PWM_PIN, 0);
    int off_ms = period_ms - on_ms;
    if (off_ms > 0) delay(off_ms);
  }
  allOff();
}

void playPattern_VaryStep(const float* pat, size_t len) {
  stopRequested = false;
  for (size_t i = 0; i < len; ++i) {
    if (stopRequested || checkStop()) break;
    int step_ms = (i <= 11) ? 500 : 250;  // <- your rule here
    ledcWrite(PWM_PIN, int(constrain(pat[i],0.f,1.f) * MAX_DUTY));
    delay(step_ms);
  }
  ledcWrite(PWM_PIN, 0);
}


void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nESP32 Pattern Controller (AP + UDP)");

  pinMode(PWM_PIN, OUTPUT);
  ledcAttach(PWM_PIN, PWM_FREQ, PWM_BITS);
    pinMode(PWM_PINL, OUTPUT);
  ledcAttach(PWM_PINL, PWM_FREQ, PWM_BITS);
  allOff();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.printf("AP: %s  PASS: %s  IP: %s\n",
    AP_SSID, AP_PASS, WiFi.softAPIP().toString().c_str());

  Udp.begin(UDP_PORT);
  Serial.printf("Listening UDP %d\n", UDP_PORT);
}

void loop() {
  int n = Udp.parsePacket();
  if (n <= 0) {
    delay(5);
    return; // idle
  }

  char buf[32];
  int len = Udp.read(buf, sizeof(buf)-1);
  if (len <= 0) return;
  buf[len] = 0;
  for (int i=0; buf[i]; ++i) buf[i] = toupper((unsigned char)buf[i]);

  Serial.printf("CMD from %s:%u -> %s\n",
    Udp.remoteIP().toString().c_str(), Udp.remotePort(), buf);

  if (strcmp(buf,"1")==0 || strcmp(buf,"P1")==0) {
    sendAck("ACK:P1");
    // playPattern(pattern1, sizeof(pattern1)/sizeof(pattern1[0]), 1000);
    playPatternRL(pattern1,pattern1Left, sizeof(pattern1)/sizeof(pattern1[0]),1000);
  } else if (strcmp(buf,"2")==0 || strcmp(buf,"P2")==0) {
    sendAck("ACK:P2");
    playPattern(pattern2, sizeof(pattern2)/sizeof(pattern2[0]), 100);
  } else if (strcmp(buf,"3")==0 || strcmp(buf,"P3")==0) {
    sendAck("ACK:P3");
    // playPattern(pattern3, sizeof(pattern3)/sizeof(pattern3[0]), 500);
    playPattern_VaryStep(pattern3, sizeof(pattern3)/sizeof(pattern3[0]));
  } else if (strcmp(buf,"4")==0 || strcmp(buf,"P4")==0 || strcmp(buf,"BOOST")==0) {
    sendAck("ACK:P4");
    patternBoost();
  } else if (strcmp(buf,"S")==0 || strcmp(buf,"STOP")==0) {
    stopRequested = true;
    allOff();
    sendAck("ACK:STOP");
  } else {
    sendAck("ERR:UNKNOWN");
  }
}
