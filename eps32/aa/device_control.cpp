#include "device_control.h"
#include "config.h"
#include "display.h"

void setupDeviceControl() {
    // Setup fan control
    pinMode(FAN_PIN, OUTPUT);
    analogWrite(FAN_PIN, 0);  // Start with fan off

    // Setup light controls
    pinMode(LIGHT1_PIN, OUTPUT);
    pinMode(LIGHT2_PIN, OUTPUT);
    digitalWrite(LIGHT1_PIN, LOW);
    digitalWrite(LIGHT2_PIN, LOW);
}

void handleDeviceControl(const JsonDocument& doc) {
    // Parse fan control commands
    if (doc["device_control"].containsKey("fan")) {
        int speed = doc["device_control"]["fan"];
        // Ensure speed is within valid range (0-255)
        if (speed >= 0 && speed <= 255) {
            fanSpeed = speed;
            analogWrite(FAN_PIN, fanSpeed);
            Serial.printf("Fan speed set to %d\n", fanSpeed);
        }
    }

    // Parse light 1 control commands
    if (doc["device_control"].containsKey("light1")) {
        bool state = doc["device_control"]["light1"];
        light1Status = state;
        digitalWrite(LIGHT1_PIN, light1Status ? HIGH : LOW);
        Serial.printf("Light 1 set to %s\n", light1Status ? "ON" : "OFF");
    }

    // Parse light 2 control commands
    if (doc["device_control"].containsKey("light2")) {
        bool state = doc["device_control"]["light2"];
        light2Status = state;
        digitalWrite(LIGHT2_PIN, light2Status ? HIGH : LOW);
        Serial.printf("Light 2 set to %s\n", light2Status ? "ON" : "OFF");
    }

    // Update the device status display if we're in the right display mode
    if (currentLcdState == NORMAL_OPERATION && currentDisplayPage == 1) {
        displayDeviceStatus();
    }

    // Send status update back to server
    extern void updateDeviceStatus();
    updateDeviceStatus();
}