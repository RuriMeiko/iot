/*
 * Smart Environment Monitoring System
 *
 * This system monitors temperature, humidity, and motion, displaying data on an LCD
 * and sending it to a cloud service. It uses a captive portal for easy WiFi setup.
 *
 * Components:
 * - ESP32 microcontroller
 * - DHT11 temperature/humidity sensor
 * - PIR motion sensor
 * - I2C LCD display (16x2)
 * - Status LED
 */

// Include all the necessary header files
#include "config.h"
#include "characters.h"
#include "display.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "api_client.h"



void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("==============================================================");
    Serial.println("Smart Environment Hub - Starting up...");
    Serial.println("==============================================================");
    Serial.println("Version: 1.0");
    Serial.printf("Build date: %s %s\n", __DATE__, __TIME__);
    Serial.println("Debug mode: ENABLED");

    // Initialize LCD
    initDisplay();

    // Welcome splash screen
    displayWelcomeScreen();

    // Initialize sensors
    initSensors();

    Serial.println("Blinking LED to indicate startup...");
    // Blink LED to indicate startup
    for(int i = 0; i < 3; i++) {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }

    // Try to connect to WiFi or start Captive Portal
    setupWiFiConnection();
    
    Serial.println("Setup complete, entering main loop");
}

void loop() {
    // Handle DNS server for Captive Portal
    dnsServer.processNextRequest();

    // Check motion sensor
    if (checkMotion()) {
        // Only update LCD immediately for motion events in normal operation
        if (currentLcdState == NORMAL_OPERATION) {
            float temp = readTemperature();
            float hum = readHumidity();
            displaySensorData(isWiFiConnected, motionDetected, isApiConnected, temp, hum);
        }
    }

    // Update motion status and LED
    updateMotionStatus();
    
    // If motion just ended, update LCD
    static bool prevMotionState = false;
    if (prevMotionState != motionDetected) {
        prevMotionState = motionDetected;
        if (!motionDetected && currentLcdState == NORMAL_OPERATION) {
            float temp = readTemperature();
            float hum = readHumidity();
            displaySensorData(isWiFiConnected, motionDetected, isApiConnected, temp, hum);
        }
    }

    // Regular LCD updates
    if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
        Serial.println("Regular LCD update");
        
        if (currentLcdState == NORMAL_OPERATION) {
            float temp = readTemperature();
            float hum = readHumidity();
            displaySensorData(isWiFiConnected, motionDetected, isApiConnected, temp, hum);
        } else {
            updateLCD(currentLcdState);
        }
        
        lastLCDUpdate = millis();
    }

    // Loading animation updates (faster than LCD)
    if (millis() - lastAnimationUpdate >= ANIMATION_INTERVAL &&
        (currentLcdState == CONNECTING_WIFI || currentLcdState == STARTING)) {
        displayLoadingAnimation();
        lastAnimationUpdate = millis();
    }

    // Send data to server
    if (isWiFiConnected && millis() - lastDataSend >= DATA_SEND_INTERVAL) {
        Serial.println("Time to send data to server");
        float temp = readTemperature();
        float hum = readHumidity();
        sendDataToServer(temp, hum, motionDetected);
        lastDataSend = millis();
    }

    // Check WiFi connection
    if (!checkWiFiStatus() && currentLcdState != AP_MODE) {
        Serial.println("WiFi connection check failed, device is in captive portal mode");
    }

    // Small delay to prevent CPU hogging
    delay(10);
}
