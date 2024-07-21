#include <esp_now.h>
#include <WiFi.h>

// Pin for motor control
const int AIN1 = 18;
const int AIN2 = 17;
const int BIN1 = 21;
const int BIN2 = 22;

// Pin for PWM control
const int PWMA = 23;
const int PWMB = 16;
const int STBY = 19;

// Holds button state data
typedef struct {
  bool FrontButton;
  bool BackButton;
  bool LeftButton;
  bool RightButton;
} ButtonMessage;

ButtonMessage Button;

// Receives button data
void OnButtonDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&Button, incomingData, sizeof(Button));
}

void setup() {
  Serial.begin(115200);

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
  esp_now_register_recv_cb(OnButtonDataRecv);
}

// Controls motors based on button push
void button() {
  if (Button.FrontButton == HIGH) {
    // Front
    Serial.println("FRONT");

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 255);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, 255);
  }

  else if (Button.LeftButton == HIGH) {
    // Left
    Serial.println("LEFT");

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 0);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, 255);
  }

  else if (Button.BackButton == HIGH) {
    // Back
    Serial.println("BACK");

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, 255);

    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, 255);
  }

  else if (Button.RightButton == HIGH) {
    // Right
    Serial.println("RIGHT");

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 255);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, 0);
  }

  else {
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
  button();
  delay(100);
}