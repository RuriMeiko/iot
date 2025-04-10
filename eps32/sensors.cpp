/*
 * File: sensors.cpp
 * Mu00f4 tu1ea3: Triu1ec3n khai cu00e1c hu00e0m quu1ea3n lu00fd cu1ea3m biu1ebfn
 */

#include "sensors.h"

// Khu1edfi tu1ea1o u0111u1ed1i tu01b0u1ee3ng DHT
DHT dht(DHTPIN, DHTTYPE);

// Biu1ebfn lu01b0u tru1ea1ng thu00e1i cu1ea3m biu1ebfn vu00e0 giu00e1 tru1ecb u0111o u0111u01b0u1ee3c
boolean motionDetected = false;        // Cu00f3 phu00e1t hiu1ec7n chuyu1ec3n u0111u1ed9ng hay khu00f4ng
unsigned long motionStartTime = 0;     // Thu1eddi u0111iu1ec3m bu1eaft u0111u1ea7u phu00e1t hiu1ec7n chuyu1ec3n u0111u1ed9ng
float currentTemperature = 0;          // Nhiu1ec7t u0111u1ed9 hiu1ec7n tu1ea1i
float currentHumidity = 0;             // u0110u1ed9 u1ea9m hiu1ec7n tu1ea1i

void setupSensors() {
  // Cu1ea5u hu00ecnh chu00e2n cho cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng (INPUT)
  pinMode(MOTION_PIN, INPUT);
  
  // Cu1ea5u hu00ecnh chu00e2n cho u0111u00e8n LED (OUTPUT) vu00e0 tu1eaft led
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Khu1edfi tu1ea1o cu1ea3m biu1ebfn DHT
  dht.begin();
  
  Serial.println("Cu1ea3m biu1ebfn DHT11 vu00e0 cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng u0111u00e3 u0111u01b0u1ee3c khu1edfi tu1ea1o");
}

void handleSensors() {
  // === Xu1eed lu00fd cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng ===
  int motionState = digitalRead(MOTION_PIN);
  
  // Nu1ebfu phu00e1t hiu1ec7n chuyu1ec3n u0111u1ed9ng vu00e0 chu01b0a ghi nhu1eadn tru01b0u1edbc u0111u00f3
  if (motionState == HIGH && !motionDetected) {
    motionDetected = true;
    motionStartTime = millis();
    digitalWrite(LED_PIN, HIGH);  // Bu1eadt u0111u00e8n LED
    
    Serial.println("Phu00e1t hiu1ec7n chuyu1ec3n u0111u1ed9ng! u0110u00e3 bu1eadt u0111u00e8n LED");
  }
  
  // Nu1ebfu u0111u00e3 phu00e1t hiu1ec7n chuyu1ec3n u0111u1ed9ng vu00e0 u0111u00e3 qua 5 giu00e2y
  if (motionDetected && (millis() - motionStartTime > 5000)) {
    motionDetected = false;
    digitalWrite(LED_PIN, LOW);  // Tu1eaft u0111u00e8n LED
    
    Serial.println("5 giu00e2y u0111u00e3 tru00f4i qua, tu1eaft u0111u00e8n LED");
  }
  
  // === u0110u1ecdc du1eef liu1ec7u tu1eeb DHT11 ===
  // DHT11 cu1ea7n thu1eddi gian u0111u1ec3 u0111u1ecdc chu00ednh xu00e1c, u0111u1ecdc mu1ed7i 2 giu00e2y
  static unsigned long lastDHTRead = 0;
  
  if (millis() - lastDHTRead > 2000) {
    lastDHTRead = millis();
    
    // u0110u1ecdc u0111u1ed9 u1ea9m
    float newHumidity = dht.readHumidity();
    // u0110u1ecdc nhiu1ec7t u0111u1ed9 (Celsius)
    float newTemperature = dht.readTemperature();
    
    // Kiu1ec3m tra xem cu00f3 u0111u1ecdc u0111u01b0u1ee3c giu00e1 tru1ecb hu1ee3p lu1ec7 hay khu00f4ng
    if (!isnan(newHumidity) && !isnan(newTemperature)) {
      currentHumidity = newHumidity;
      currentTemperature = newTemperature;
      
      Serial.print("Nhiu1ec7t u0111u1ed9: ");
      Serial.print(currentTemperature);
      Serial.print("u00b0C, u0110u1ed9 u1ea9m: ");
      Serial.print(currentHumidity);
      Serial.println("%");
    } else {
      Serial.println("Khu00f4ng thu1ec3 u0111u1ecdc du1eef liu1ec7u tu1eeb cu1ea3m biu1ebfn DHT!");
    }
  }
}

float getTemperature() {
  return currentTemperature;
}

float getHumidity() {
  return currentHumidity;
}

bool isMotionDetected() {
  return motionDetected;
}