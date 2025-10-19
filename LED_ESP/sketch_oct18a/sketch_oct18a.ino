#if !defined(ARDUINO_ARCH_ESP32)
#error "Select Tools > Board > ESP32 Arduino > ESP32 Dev Module"
#endif

const int PWM_PIN  = 25;      // output to transistor base
const int PWM_FREQ = 2000;    // 2 kHz
const int PWM_BITS = 8;       // 8-bit resolution (0-255)
const int MAX_DUTY = 255;

// pattern: 40% → 90% peaks → back down (normalized 0–1.0)
float pattern[] = {
  0.0, 0.0, 0.0, 0.0, 0.2, 0.4, 0.5, 0.6, 0.8, 0.9, 0.9, 0.8, 0.7, 0.6,
  0.5, 0.6, 0.8, 0.9, 0.8, 0.7, 0.5, 0.6, 0.7, 0.75, 0.85, 0.7,0.6,0.0,0.0,0.0,0.0
};
float pattern2[] = {
  0.0, 0.0, 0.0, 1,1,1,1,0,0,0,0
};
float pattern3[] = {
  0.0, 0.15, 0.3, 0.0,0.3,0,0.3,0,0.3,0,0.3,0,0.9,0,0.9,0,0.9,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

int N = sizeof(pattern) / sizeof(pattern[0]);

int STEP_MS = 1000;  // time per step ≈ matches your chart spacing

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nPWM pattern playback (40%→90%→down)");
  pinMode(PWM_PIN, OUTPUT);
  ledcAttach(PWM_PIN, PWM_FREQ, PWM_BITS);
}

void loop() {
  Serial.println("--- Pattern Start ---");
  N = sizeof(pattern) / sizeof(pattern[0]);
  STEP_MS = 1000;
  for (int i = 0; i < N; i++) {
    float p = pattern[i];
   // if (p < 0.4f) p = 0.4f;   // enforce min 40%
   // if (p > 0.9f) p = 0.9f;   // cap at 90%

    int duty = int(p * MAX_DUTY);
    ledcWrite(PWM_PIN, duty);

    float volt = 3.3f * duty / MAX_DUTY;
    Serial.printf("t=%2ds  duty=%3d (%.0f%%)  ≈%.2f V\n", i, duty, p * 100, volt);

    delay(STEP_MS);
  }
  ledcWrite(PWM_PIN, 0);  // off at end
  Serial.println("--- Pattern End ---\n");
  delay(10000);  // rest before repeating


N = sizeof(pattern2) / sizeof(pattern2[0]);

 Serial.println("--- Pattern2 Start ---");
  for (int i = 0; i < N; i++) {
    float p = pattern2[i];
   // if (p < 0.4f) p = 0.4f;   // enforce min 40%
   // if (p > 0.9f) p = 0.9f;   // cap at 90%

    int duty = int(p * MAX_DUTY);
    ledcWrite(PWM_PIN, duty);

    float volt = 3.3f * duty / MAX_DUTY;
    Serial.printf("t=%2ds  duty=%3d (%.0f%%)  ≈%.2f V\n", i, duty, p * 100, volt);

    delay(STEP_MS);
  }
    delay(10000);  // rest before repeating
N=0;
N = sizeof(pattern3) / sizeof(pattern3[0]);
STEP_MS = 500;

   Serial.println("--- Pattern3 Start ---");
  for (int i = 0; i < N; i++) {
    float p = pattern3[i];
   // if (p < 0.4f) p = 0.4f;   // enforce min 40%
   // if (p > 0.9f) p = 0.9f;   // cap at 90%

    int duty = int(p * MAX_DUTY);
    ledcWrite(PWM_PIN, duty);

    float volt = 3.3f * duty / MAX_DUTY;
    Serial.printf("t=%2ds  duty=%3d (%.0f%%)  ≈%.2f V\n", i, duty, p * 100, volt);
    if(i==11) STEP_MS = 250;
    delay(STEP_MS);
  }
    delay(10000);  // rest before repeating
}
