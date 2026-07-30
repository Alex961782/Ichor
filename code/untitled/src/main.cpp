#include <ESP32Servo.h>
#include <PS4Controller.h>
#include <Arduino.h>

// Right Wheel
int rightWheelREN = 14;
int rightWheelLEN = rightWheelREN;
int rightWheelLPWM = 26;
int rightWheelRPWM = 27;

// Left Wheel
int leftWheelREN = 19;
int leftWheelLEN = leftWheelREN;
int leftWheelRPWM = 18;
int leftWheelLPWM = 5;

// Right Drill Motor
int rightDrumDrillREN = 17;
int rightDrumDrillLEN = rightDrumDrillREN;
int rightDrumDrillRPWM = 16;
int rightDrumDrillLPWM = 4;

// Left Drill Motor
int leftDrumDrillREN = 25;
int leftDrumDrillLEN = leftDrumDrillREN;
int leftDrumDrillRPWM = 33;
int leftDrumDrillLPWM = 32;

// Servo Motors
Servo rightServo;
Servo leftServo;
Servo sensorServo;
int sensorServoPin = 21;
int rightServoPin = 22;
int leftServoPin = 23;
int sensorCounter = 90;
int counter = 90;

// Soil Moisture Sensor
const int soilPin = 36;

// Setting PWM properties
const int freq = 5000;
const int resolution = 8;
int dutyCycle = 200;

// PWM Channels dedicated to each motor.
const int rightWheelRC = 4;
const int rightWheelLC = 5;
const int leftWheelRC = 6;
const int leftWheelLC = 7;
const int rightDrumDrillRC = 8;
const int rightDrumDrillLC = 9;
const int leftDrumDrillRC = 10;
const int leftDrumDrillLC = 11;

// PWM Speeds
const int MAX_SPEED = 255;
const int ROAMING_SPEED = 200;
const int HALF_SPEED = 125;
bool isMaxSpeed = false;

// Method to set up motors.
void setupMotor(int ren, int len, int rPWM, int lPWM, int rc, int lc);

// Methods to move the motors.
void forward(int rightChannel, int leftChannel, int speed);
void reverse(int rightChannel, int leftChannel, int speed);
void stopMotor(int rightChannel, int leftChannel);
void moveServos(int);

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(100);


    // Timing for Channels
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    // Servo Motor 1
    rightServo.setPeriodHertz(50);          // Standard servo frequency
    rightServo.attach(rightServoPin, 500, 2500); // Min/max pulse width in microseconds
    rightServo.write(90);

    // Servo Motor 2
    leftServo.setPeriodHertz(50);          // Standard servo frequency
    leftServo.attach(leftServoPin, 500, 2500); // Min/max pulse width in microseconds
    leftServo.write(90);

    // Sensor Motor
    sensorServo.setPeriodHertz(50);          // Standard servo frequency
    sensorServo.attach(sensorServoPin, 500, 2500); // Min/max pulse width in microseconds
    sensorServo.write(90);

    // Right Wheel Setup
    setupMotor(rightWheelREN, rightWheelLEN, rightWheelRPWM, rightWheelLPWM,rightWheelRC,rightWheelLC);

    // Left Wheel Setup
    setupMotor(leftWheelREN, leftWheelLEN, leftWheelRPWM, leftWheelLPWM,leftWheelRC, leftWheelLC);

    // Right Drill Motor Setup
    setupMotor(rightDrumDrillREN, rightDrumDrillLEN, rightDrumDrillRPWM, rightDrumDrillLPWM,rightDrumDrillRC,rightDrumDrillLC);

    // Left Drill Motor Setup
    setupMotor(leftDrumDrillREN, leftDrumDrillLEN, leftDrumDrillRPWM, leftDrumDrillLPWM, leftDrumDrillRC, leftDrumDrillLC);

    // PS4 Controller Communication
    PS4.begin("8c:94:df:95:83:84");
    Serial.println("Ready to connect to PS4 controller.");

    // Soil Moisture Sensor Setup
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

void loop() {

    int moisture = analogRead(soilPin);

    //Serial.println(moisture);
    int percent = map(moisture, 3500, 1200, 0, 100);
    percent = constrain(percent, 0, 100);

    if (PS4.isConnected())
    {
        // Wheel Movement
        if (PS4.Up())
        {
            forward(rightWheelRC, rightWheelLC, MAX_SPEED);
            forward(leftWheelRC, leftWheelLC, MAX_SPEED);
        }
        else if (PS4.Down()) {
            Serial.println("Motor moving backward");

            reverse(rightWheelRC, rightWheelLC, MAX_SPEED);
            reverse(leftWheelRC, leftWheelLC, MAX_SPEED);
        }
        else if (PS4.Left()) {
            Serial.println("Motor moving left");

            forward(rightWheelRC, rightWheelLC, MAX_SPEED);
            reverse(leftWheelRC, leftWheelLC, MAX_SPEED);
        }
        else if (PS4.Right()) {
            Serial.println("Motor moving right");
            reverse(rightWheelRC, rightWheelLC, MAX_SPEED);
            forward(leftWheelRC, leftWheelLC, MAX_SPEED);
            //stopMotors(motor1Pin1, motor1Pin2);
        }
        else {
            //Serial.println("Motor stopped");
            stopMotor(rightWheelRC, rightWheelLC);
            stopMotor(leftWheelRC, leftWheelLC);
        } // End Wheel Movement


        // Servo Motor Movement
        if (PS4.R1()) {
            counter -= 2;
            if (counter < 0) counter = 0;

            Serial.println(counter);
            moveServos(counter);
        }
        if (PS4.L1()) {
            counter += 2;
            if (counter > 170) counter = 170;

            Serial.println(counter);
            moveServos(counter);
        } // Servo Motor End

        // Sensor Servo Motor Movement
        if (PS4.R3()) {
            sensorCounter -= 2;
            if (sensorCounter < 0) sensorCounter = 0;

            Serial.println(sensorCounter);
            sensorServo.write(sensorCounter);
        }
        if (PS4.L3()) {
            sensorCounter += 2;
            if (sensorCounter > 170) sensorCounter = 170;

            Serial.println(sensorCounter);
            sensorServo.write(sensorCounter);
        } // Sensor Servo Motor End

        // Drill Movement
        if (PS4.Cross()) {
            Serial.println("Drill Movement: Dig");
            forward(rightDrumDrillRC, rightDrumDrillLC, HALF_SPEED);
            forward(leftDrumDrillRC, leftDrumDrillLC, HALF_SPEED);
        }
        else if (PS4.Square()) {
            Serial.println("Drill Movement: Release");
            reverse(rightDrumDrillRC, rightDrumDrillLC, HALF_SPEED);
            reverse(leftDrumDrillRC, leftDrumDrillLC, HALF_SPEED);
        }
        else {
            stopMotor(rightDrumDrillRC, rightDrumDrillLC);
            stopMotor(leftDrumDrillRC, leftDrumDrillLC);
        }

        // Soil Moisture Activation
        if (PS4.Triangle())
        {
            if (percent > 60)
            {
                Serial.println("Good Soil!");
                PS4.setRumble(255, 255);
                PS4.sendToController();
                delay(200);
                PS4.setRumble(0, 0);
                PS4.sendToController();
                delay(100);
                PS4.setRumble(255, 255);
                PS4.sendToController();
                delay(200);
                PS4.setRumble(0, 0);
                PS4.sendToController();
            }
            else
            {
                Serial.println("Bad Soil!");
                PS4.setRumble(0, 255);
                PS4.sendToController();
                delay(100);
                PS4.setRumble(0, 0);
                PS4.sendToController();
                delay(100);
            }
        } // Soil Moisture End
    }

    delay(10);
}

void forward(int rightChannel, int leftChannel, int speed) {
    ledcWrite(rightChannel, 0);
    ledcWrite(leftChannel, speed);
}

void reverse(int rightChannel, int leftChannel, int speed) {
    ledcWrite(rightChannel, speed);

    ledcWrite(leftChannel, 0);
}

void stopMotor(int rightChannel, int leftChannel) {
    ledcWrite(rightChannel, 0);
    ledcWrite(leftChannel, 0);
}

void moveServos(int angle) {
    leftServo.write(angle);
    rightServo.write(angle);

    // Force the ESP32 to finish the commands before continuing
    delayMicroseconds(500);
}

void setupMotor(int ren, int len, int rpwm, int lPWM, int rc, int lc) {
    // Setting up the enable pins for output.
    pinMode(ren, OUTPUT);
    pinMode(len, OUTPUT);

    // Setting the enable pins to ON.
    digitalWrite(ren, HIGH);
    digitalWrite(len, HIGH);

    // Setting up how fast the signal is received.
    ledcSetup(rc, freq, resolution);
    ledcSetup(lc, freq, resolution);

    // Connecting the Motor's PWM to their respective PWM channel.
    ledcAttachPin(rpwm, rc);
    ledcAttachPin(lPWM, lc);
}
