#include "motionModel2.h"
#include <tflm_esp32.h>                              
#include <eloquent_tinyml.h>                           
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <esp_now.h>
#include <WiFi.h>

#define ARENA_SIZE 10000

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Broadcast address for ESP-NOW communication
uint8_t broadcastAddress[] = {0xE0, 0xE2, 0xE6, 0x3D, 0xB2, 0x0C};

// Callback function for ESP-NOW data transmission
esp_now_peer_info_t peerInfo;

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

// Initialize SSD1306 display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire,-1);

// Initialize MPU6050 sensor
Adafruit_MPU6050 mpu;

// TensorFlow model instance
Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;

// Pin definitions
const int buttonPin = 32;
const int ledPin1 = 26;

// Stores sensor data and gesture prediction
int samples = 0;
int gesture_detected;
float s_data[600]; 

// ESP-NOW data transmission
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  // Initialize MPU6050 sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  // Sets sensor range and filter bandwidth
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);

  // Initialize SSD1306 display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Clears display
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  // Set pin modes
  pinMode(buttonPin, INPUT);
  pinMode(ledPin1, OUTPUT);

  // Initialize TensorFlow model
  Serial.println("__LOAD TENSORFLOW GESTURE DETECTION MODEL__");
  while (!tf.begin(motionModel2).isOk())
    Serial.println(tf.exception.toString());

  // Initialize WiFi
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Configure peer information
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {

  // Checks if button is pressed
  if (digitalRead(buttonPin) == HIGH) {
    while (digitalRead(buttonPin) == HIGH) {
      delay(10);
    }

    samples = 0;

    // Collect sensor data
    while (samples <= 99) {
      
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      digitalWrite(ledPin1, HIGH);

      s_data[samples] = a.acceleration.x;
      s_data[100+samples] = a.acceleration.y;
      s_data[200+samples] = a.acceleration.z;
      s_data[300+samples] = g.gyro.x;
      s_data[400+samples] = g.gyro.y;
      s_data[500+samples] = g.gyro.z;
      delay(10);
      samples = samples + 1;
    }
    
    digitalWrite(ledPin1, LOW);

    // Prints collected sensor data
    for (int i = 0; i < 600; i++){
      Serial.print(s_data[i]);
      Serial.print("   ");
    }
    Serial.println();
    delay(5);

    // Predict gesture based on sensor data
    if (!tf.predict(s_data).isOk()) {
      Serial.println(tf.exception.toString());
      return;
    }
    Serial.print("Predicting motion based on sensor data: ");
    Serial.println(tf.classification);

    gesture_detected = tf.classification;

    // Displays detected gesture on SSD1306 display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Gesture Detected: ");
    display.print(gesture_detected);
    display.display(); 

    // Maps gesture to MotionEvent structure
    Motion.SpinMotion = (gesture_detected == 0);
    Motion.BackMotion = (gesture_detected == 1);  
    Motion.FrontMotion = (gesture_detected == 2);
    Motion.LeftMotion = (gesture_detected == 3);  
    Motion.RightMotion = (gesture_detected == 4);
    Motion.StopMotion = (gesture_detected == 5);

    // Sends motion event data over ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&Motion, sizeof(Motion));
    if (result != ESP_OK) {
      Serial.println("Error sending data over ESP-NOW");
    }
    delay(1000);
  }
}
