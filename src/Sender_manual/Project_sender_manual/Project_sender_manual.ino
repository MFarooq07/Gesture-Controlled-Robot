#include <esp_now.h>
#include <WiFi.h>

// MAC address of receiver ESP32
uint8_t broadcastAddress[] = {0xE0, 0xE2, 0xE6, 0x3D, 0xB2, 0x0C};

// Variable to add info about peer
esp_now_peer_info_t peerInfo;

const int frontButton = 4;
const int backButton = 18;
const int leftButton = 19;
const int rightButton = 32;

String success;

// Defines the format of the message sent over ESP-NOW
typedef struct {
  bool FrontButton; 
  bool BackButton; 
  bool LeftButton; 
  bool RightButton;
} ButtonMessage;

ButtonMessage Button;

// Handles the status of data transmission over ESP-NOW
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {

  // Set button pins as inputs
  pinMode(frontButton, INPUT);
  pinMode(backButton, INPUT);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);

  Serial.begin(115200);

  // Initialize WiFi
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Registers callback function to handle data transmission status
  esp_now_register_send_cb(OnDataSent);

  // Configures peer settings
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Adds peer 
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {

  // Check the state of each button and updates corresponding ButtonMessage 
  // Front
  if (digitalRead(frontButton) == HIGH && !Button.FrontButton){
    Button.FrontButton = true;
    Serial.println("FRONT");
  }
  else if (digitalRead(frontButton) == LOW && Button.FrontButton){
    Button.FrontButton = false;
  }

  // Back
  if (digitalRead(backButton) == HIGH && !Button.BackButton){
    Button.BackButton = true;
    Serial.println("BACK");
  }
  else if (digitalRead(backButton) == LOW && Button.BackButton){
    Button.BackButton = false;
  }
  
  // Left
  if (digitalRead(leftButton) == HIGH && !Button.LeftButton){
    Button.LeftButton = true;
    Serial.println("LEFT");
  }
  else if (digitalRead(leftButton) == LOW && Button.LeftButton){
    Button.LeftButton = false;
  }
  
  // Right
  if (digitalRead(rightButton) == HIGH && !Button.RightButton){
    Button.RightButton = true;
    Serial.println("RIGHT");
  }
  else if (digitalRead(rightButton) == LOW && Button.RightButton){
    Button.RightButton = false;
  }

  // Sends the ButtonMessage structure over ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Button, sizeof(Button));

  delay(100); 
}

