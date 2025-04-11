/*
 * Sensors management implementation for Smart Environment Monitoring System
 */

#include "sensors.h"

DHT dht(DHTPIN, DHTTYPE);


void initSensors() {
    Serial.println("Initializing sensors...");
    dht.begin();
    pinMode(PIRPIN, INPUT);
    pinMode(LED, OUTPUT);
    
    Serial.println("Sensors initialized.");
    Serial.println("DHT sensor type: DHT11 on pin " + String(DHTPIN));
    Serial.println("Motion sensor on pin " + String(PIRPIN));
    Serial.println("LED indicator on pin " + String(LED));

}

float readTemperature() {
    float temp = dht.readTemperature();
    Serial.printf("Temperature reading: %.2f°C\n", temp);
    if (isnan(temp)) {
        Serial.println("ERROR: Failed to read temperature from DHT sensor!");
    }
    return temp;
}

float readHumidity() {
    float humidity = dht.readHumidity();
    Serial.printf("Humidity reading: %.2f%%\n", humidity);
    if (isnan(humidity)) {
        Serial.println("ERROR: Failed to read humidity from DHT sensor!");
    }
    return humidity;
}

bool checkMotion() {
    int motionState = digitalRead(PIRPIN);
    if (motionState == HIGH) {
        motionDetected = true;
        lastMotionTime = millis();
        digitalWrite(LED, HIGH);
        Serial.println("Motion detected!");
        return true;
    }
    return false;
}

void updateMotionStatus() {
    if (motionDetected && (millis() - lastMotionTime >= MOTION_TIMEOUT)) {
        motionDetected = false;
        digitalWrite(LED, LOW);
        Serial.println("Motion timeout - turning off LED");
    }
}
