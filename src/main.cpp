#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

int relayPin = 12;
int moistureSensorPin = A2;

long lastCheck = 0;
const long interval = 4000; // interval to check moisture (4 seconds)

// OLED screen object
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  Serial.begin(9600);

  pinMode(relayPin, OUTPUT); 

  digitalWrite(relayPin, HIGH); // Relay OFF from start
  u8g2.begin();
}

void loop() {

  if (millis() - lastCheck >= interval) {
    lastCheck = millis();

    int sensorRead = analogRead(moistureSensorPin);
    Serial.println(sensorRead);

    // OLED display showing moisture level
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(20, 20, "Moisture Level:");
    u8g2.setCursor(55, 40);
    u8g2.print(sensorRead);
    u8g2.sendBuffer();

    // Pump control
    if (sensorRead < 300) {
      digitalWrite(relayPin, LOW); // ON
      
    } else {
      digitalWrite(relayPin, HIGH); // OFF
      
    }
  }
}