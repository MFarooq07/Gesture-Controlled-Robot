#include <esp_now.h>
#include <WiFi.h>

const int buttonPin = 33;

// Pin for motor control
const int AIN1 = 18;
const int AIN2 = 17;
const int BIN1 = 21;
const int BIN2 = 22;

// Pin for PWM control
const int PWMA = 23;
const int PWMB = 16;
const int STBY = 19;

// Holds motion event data
typedef struct {
  bool SpinMotion;
  bool BackMotion;
  bool FrontMotion;
  bool LeftMotion;
  bool RightMotion;
  bool StopMotion;
} MotionEvent;

MotionEvent Motion;

// Receives motion data
void OnMotionDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&Motion, incomingData, sizeof(Motion));
}

void setup() {
  Serial.begin(115200);

  pinMode(33, INPUT);

  // Pin modes for motor control
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  // Pin modes for PWM control
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Put ON the motor drive, set to LOW to put OFF
  digitalWrite(STBY, HIGH);

  // Initialize WiFi
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Callback functions for receiving data
  esp_now_register_recv_cb(OnMotionDataRecv);
}


// Controls motors based on motion events
void motion() {
  if (Motion.FrontMotion == HIGH) {
    // Front
    Serial.println("FRONT");

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 255);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, 255);
  }

  else if (Motion.LeftMotion == HIGH) {
    // Left
    Serial.println("LEFT");

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 0);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, 255);
  }

  else if (Motion.BackMotion == HIGH) {
    // Back
    Serial.println("BACK");

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, 255);

    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, 255);
  }

  else if (Motion.RightMotion == HIGH) {
    // Right
    Serial.println("RIGHT");

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 255);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, 0);
  }

  else if (Motion.SpinMotion == HIGH) {
    // Spin to the right
    Serial.println("SPIN");

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 255);

    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, 255);
  }

  else if (Motion.StopMotion == HIGH) {
    // Stop
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 0);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, 0);
  }
}

void loop() {
  motion();
  delay(100);
}
