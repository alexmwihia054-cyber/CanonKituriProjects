#include <Servo.h>

const int irPin = A0;
const int servo1Pin = 9;
const int servo2Pin = 10;
const int relayPin = 8;

const int threshold = 500;
const int confirmCount = 5;
const int sweepStep = 2;
const int sweepDelay = 15;

Servo sweepServo;
Servo hoseServo;

int fireAngle = 0;
bool fireDetected = false;
int direction = 1;
int currentPos = 0;

unsigned long previousMillis = 0;

void setup() {
  pinMode(irPin, INPUT);
  pinMode(relayPin, OUTPUT);
  sweepServo.attach(servo1Pin);
  hoseServo.attach(servo2Pin);
  digitalWrite(relayPin, LOW);
}

void loop() {

  if (!fireDetected) {

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= sweepDelay) {
      previousMillis = currentMillis;

      sweepServo.write(currentPos);

      int fireReadings = 0;

      for (int i = 0; i < confirmCount; i++) {
        if (analogRead(irPin) > threshold) {
          fireReadings++;
        }
      }

      if (fireReadings == confirmCount) {
        fireDetected = true;
        fireAngle = currentPos;
      }

      currentPos += sweepStep * direction;

      if (currentPos >= 180) {
        currentPos = 180;
        direction = -1;
      }

      if (currentPos <= 0) {
        currentPos = 0;
        direction = 1;
      }
    }

  } else {

    sweepServo.write(fireAngle);
    hoseServo.write(fireAngle);

    digitalWrite(relayPin, HIGH);

    while (analogRead(irPin) > threshold) {
    }

    digitalWrite(relayPin, LOW);

    fireDetected = false;
  }
}
