#include "sensors.h"
#include "config.h"
#include "display.h"
#include "websocket_handler.h"
#include <WiFi.h>
#include <ArduinoJson.h>

// Define DHT variable as extern since it's defined in aa.ino
extern DHT dht;

void setupSensors() {
    dht.begin();
    pinMode(PIRPIN, INPUT);
    pinMode(LED, OUTPUT);

    // Blink LED to indicate sensor setup
    for(int i = 0; i < 2; i++) {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }
}

void checkMotion() {
    // Check motion sensor
    if (digitalRead(PIRPIN) == HIGH) {
        motionDetected = true;
        lastMotionTime = millis();
        digitalWrite(LED, HIGH);

        // Only update LCD immediately for motion events in normal operation
        if (currentLcdState == NORMAL_OPERATION && currentDisplayPage == 0) {
            updateLCD();
        }

        // Send motion event immediately through WebSocket if connected
        if (isWsConnected) {
            JsonDocument doc;
            doc["event"] = "motion_detected";
            doc["device_id"] = WiFi.macAddress();

            String jsonPayload;
            serializeJson(doc, jsonPayload);
            webSocket.sendTXT(jsonPayload);
        }
    }

    // Turn off LED after timeout
    if (motionDetected && (millis() - lastMotionTime >= MOTION_TIMEOUT)) {
        motionDetected = false;
        digitalWrite(LED, LOW);

        // Update LCD when motion stops
        if (currentLcdState == NORMAL_OPERATION && currentDisplayPage == 0) {
            updateLCD();
        }

        // Send motion stopped event through WebSocket if connected
        if (isWsConnected) {
            JsonDocument doc;
            doc["event"] = "motion_stopped";
            doc["device_id"] = WiFi.macAddress();

            String jsonPayload;
            serializeJson(doc, jsonPayload);
            webSocket.sendTXT(jsonPayload);
        }
    }
}