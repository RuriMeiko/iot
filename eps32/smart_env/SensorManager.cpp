#include "SensorManager.h"

SensorManager::SensorManager(uint8_t dhtPin, uint8_t dhtType, uint8_t pirPin, uint8_t ledPin)
  : dht(dhtPin, dhtType), pirPin(pirPin), ledPin(ledPin) {
    motionState = false;
    lastMotionTime = 0;
}

void SensorManager::begin() {
    dht.begin();
    pinMode(pirPin, INPUT);
    pinMode(ledPin, OUTPUT);

    // Initial LED test - blink 3 times
    for(int i = 0; i < 3; i++) {
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        delay(100);
    }
}

float SensorManager::readTemperature() {
    return dht.readTemperature();
}

float SensorManager::readHumidity() {
    return dht.readHumidity();
}

bool SensorManager::detectMotion() {
    if (digitalRead(pirPin) == HIGH) {
        motionState = true;
        lastMotionTime = millis();
        digitalWrite(ledPin, HIGH);
        return true;
    }
    return false;
}

void SensorManager::updateMotionStatus(unsigned long motionTimeout) {
    if (motionState && (millis() - lastMotionTime >= motionTimeout)) {
        motionState = false;
        digitalWrite(ledPin, LOW);
    }
}

bool SensorManager::getMotionState() {
    return motionState;
}
