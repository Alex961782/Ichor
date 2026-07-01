/* ESP Transmitter test code using ESP NOW*/

#include <Arduino.h>
#include <PS4Controller.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

// Receiver Address: 1C:C3:AB:3C:9A:2C
// Actual Receiver Address: 68:09:47:28:F2:0C

uint8_t receiverAddress[] = { 0x68, 0x09, 0x47, 0x28, 0xF2, 0x0C };

bool prevL1 = false;

// Motor A
int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 14;

// Motor B

int motor2Pin1 = 33;
int motor2Pin2 = 32;
int enable2Pin = 25;

// Motor C

int motor3Pin1 = 22;
int motor3Pin2 = 23;
int enable3Pin = 21;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

typedef struct struct_message {
  int16_t l1Button;
} struct_message;

void onSent(const uint8_t *macAddr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void stopMotors(int, int);
void forwardMotors(int, int);
void backwardMotors(int, int);

void setup() {

  // sets the pins as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  pinMode(motor3Pin1, OUTPUT);
  pinMode(motor3Pin2, OUTPUT);
  pinMode(enable3Pin, OUTPUT);

  // configure LEDC PWM
  //ledcAttachChannel(enable1Pin, freq, resolution, pwmChannel);
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(enable2Pin, pwmChannel);
  ledcAttachPin(enable3Pin, pwmChannel);


  // All Communications Begins
  Serial.begin(115200);

  // PS4 Communication to Transmitter
  //PS4.begin("68:09:47:28:F2:0E");
  //PS4.begin("68:09:47:28:F2:0C"); // test esp32
  PS4.begin("8c:94:df:95:83:84"); // test esp32

  Serial.println("Ready to connect to PS4 controller.");

  
  // Transmitter to Receiver
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (1);
  }
  
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (1);
  }
}

void loop() {
  // Below has all accessible outputs from the controller
  ledcWrite(pwmChannel, 255);

  if (PS4.isConnected()) {

    // Wheel Movement
    if (PS4.Up()) {
      Serial.println("Motor moving forward");

      forwardMotors(motor1Pin1, motor1Pin2);
      forwardMotors(motor2Pin1, motor2Pin2);
    }
    else if (PS4.Down()) {
      Serial.println("Motor moving backward");

      backwardMotors(motor1Pin1, motor1Pin2);
      backwardMotors(motor2Pin1, motor2Pin2);
    }
    else if (PS4.Left()) {
      Serial.println("Motor moving left");

      forwardMotors(motor1Pin1, motor1Pin2);
      stopMotors(motor2Pin1, motor2Pin2);
    }
    else if (PS4.Right()) {
      Serial.println("Motor moving right");
      forwardMotors(motor2Pin1, motor2Pin2);
      stopMotors(motor1Pin1, motor1Pin2);
    }
    else {
      Serial.println("Motor stopped");

      stopMotors(motor1Pin1, motor1Pin2);
      stopMotors(motor2Pin1, motor2Pin2);
    }
    // End Wheel Movement

    // Drill Movement
    if (PS4.Cross()) {
      Serial.println("Drill Movement");

      forwardMotors(motor3Pin1, motor3Pin2);
    }
    else {
      stopMotors(motor3Pin1, motor3Pin2);
    }


    // if L1 is pressed down, make boolean variable true.
    // if not, make var false.

    Serial.println();
    // This delay is to make the output more human readable
    // Remove it when you're not trying to see the output
    delay(100);
  }

  // message of var
  // sent out

  // motors
}

void stopMotors(int pin1, int pin2) {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  delay(100);
}

void forwardMotors(int pin1, int pin2) {
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, LOW);
  delay(100);
}

void backwardMotors(int pin1, int pin2) {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, HIGH);
  delay(100);
}