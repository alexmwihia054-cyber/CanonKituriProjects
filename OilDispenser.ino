#define MOTOR1_EN 5
#define MOTOR1_IN1 2
#define MOTOR1_IN2 3
#define MOTOR2_EN 6
#define MOTOR2_IN3 4
#define MOTOR2_IN4 7
#define POT_PIN A0
#define TRIG_TANK 8
#define ECHO_TANK 9
#define TRIG_GLASS 10
#define ECHO_GLASS 11
#define RELAY_PIN 12
#define RGB_R 13
#define RGB_G A1
#define RGB_B A2

#define MAX_TANK_DIST 20
#define GLASS_TRIGGER 30
#define TANK_UPDATE_INTERVAL 75
#define GLASS_UPDATE_INTERVAL 50
#define SERIAL_INTERVAL 200

unsigned long prevTankMillis = 0;
unsigned long prevGlassMillis = 0;
unsigned long prevSerialMillis = 0;

unsigned int tankDistance = MAX_TANK_DIST;
unsigned int glassDistance = GLASS_TRIGGER + 10;
int motorSpeed = 0;
bool dispensing = false;

void setup() {
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN3, OUTPUT);
  pinMode(MOTOR2_IN4, OUTPUT);
  pinMode(MOTOR1_EN, OUTPUT);
  pinMode(MOTOR2_EN, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  digitalWrite(MOTOR1_IN1, HIGH);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN3, HIGH);
  digitalWrite(MOTOR2_IN4, LOW);

  digitalWrite(RELAY_PIN, LOW);

  Serial.begin(9600);
}

unsigned int readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 20000);
  unsigned int distance = duration / 58;
  if (distance == 0 || distance > MAX_TANK_DIST) distance = MAX_TANK_DIST;
  return distance;
}

void loop() {
  unsigned long currentMillis = millis();

  int targetSpeed = map(analogRead(POT_PIN), 0, 1023, 0, 255);
  motorSpeed += (targetSpeed - motorSpeed) / 4;
  analogWrite(MOTOR1_EN, motorSpeed);
  analogWrite(MOTOR2_EN, motorSpeed);

  if (currentMillis - prevTankMillis >= TANK_UPDATE_INTERVAL) {
    prevTankMillis = currentMillis;
    tankDistance = (tankDistance * 3 + readDistanceCM(TRIG_TANK, ECHO_TANK)) / 4;
  }

  if (currentMillis - prevGlassMillis >= GLASS_UPDATE_INTERVAL) {
    prevGlassMillis = currentMillis;
    glassDistance = readDistanceCM(TRIG_GLASS, ECHO_GLASS);

    if (glassDistance < GLASS_TRIGGER) {
      digitalWrite(RELAY_PIN, HIGH);
      dispensing = true;
    } else {
      digitalWrite(RELAY_PIN, LOW);
      dispensing = false;
    }
  }

  if (dispensing) {
    analogWrite(RGB_R, 255);
    analogWrite(RGB_G, 0);
    analogWrite(RGB_B, 0);
  } else {
    float fillPercent = 1.0 - ((float)tankDistance / MAX_TANK_DIST);
    fillPercent = constrain(fillPercent, 0, 1);

    int redValue = 255 * (1 - fillPercent);
    int blueValue = 255 * fillPercent;
    analogWrite(RGB_R, redValue);
    analogWrite(RGB_G, 0);
    analogWrite(RGB_B, blueValue);
  }

  if (currentMillis - prevSerialMillis >= SERIAL_INTERVAL) {
    prevSerialMillis = currentMillis;
    Serial.print("Motor PWM: "); Serial.print(motorSpeed);
    Serial.print(" | Tank cm: "); Serial.print(tankDistance);
    Serial.print(" | Glass cm: "); Serial.print(glassDistance);
    Serial.print(" | Pump: "); Serial.println(dispensing ? "ON" : "OFF");
  }
}
