#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

const int relayPin = 12;
const int moistureSensorPin = A2;
const int trigPin = 9;
const int echoPin = 10;

float distance = -1;
bool pumpOn = false;

long lastMoistureCheck = 0;
long lastUltrasonicCheck = 0;

const long moistureInterval = 4000; // 4 seconds between moisture checks to avoid excessive sensor reads
const long ultrasonicInterval = 4000; // 4 seconds check of water level

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Ultrasonic water level reader

float rawDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); 

  if (duration == 0) return -1;

  return (duration * 0.0343) / 2;
}

float readDistanceStable() { // Read distance twice for more stable reading
  float d1 = rawDistance();
  delay(15);
  float d2 = rawDistance();

  // both invalid - return -1
  if (d1 == -1 && d2 == -1) return -1;

  // one valid - use it
  if (d1 == -1) return d2;
  if (d2 == -1) return d1;

  // both valid - return average
  return (d1 + d2) / 2.0;
}

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(relayPin, HIGH); // off by default

  u8g2.begin();
}

void loop() {

  // moisture sensor check every 4 secs
  if (millis() - lastMoistureCheck >= moistureInterval) {
    lastMoistureCheck = millis();

    int sensorRead = analogRead(moistureSensorPin);

    Serial.print("Moisture: ");
    Serial.println(sensorRead);

    if (sensorRead < 300) {
      digitalWrite(relayPin, LOW);   // if moisture low - turn water pump on
      pumpOn = true;
    } else {
      digitalWrite(relayPin, HIGH);  // if moisture ok - turn water pump off
      pumpOn = false;
    }
  }

  // ultrasonic sensor check every 4 seconds

  if (!pumpOn && millis() - lastUltrasonicCheck >= ultrasonicInterval) {

    lastUltrasonicCheck = millis();

    float newDistance = readDistanceStable();

    // reject extreme spikes
    if (newDistance > 200 || newDistance < 0) {
      newDistance = -1;
    }

    distance = newDistance;

    Serial.print("Distance: ");
    Serial.println(distance);
  }

  // OLED disply setup and update
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(10, 15, "Moisture:");
  u8g2.setCursor(80, 15);
  u8g2.print(analogRead(moistureSensorPin)); // Display moisture value

  u8g2.drawStr(10, 35, "Water:"); // Display water level status

  if (distance == -1) {
    u8g2.drawStr(60, 35, "NO SIGNAL");
  } 
  else if (distance > 10) {
    u8g2.drawStr(60, 35, "LOW");
  } 
  else {
    u8g2.drawStr(60, 35, "OK");
  }

  u8g2.sendBuffer();
}