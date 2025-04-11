#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>
#include <WebSocketsClient.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

#include "config.h"
#include "custom_characters.h"
#include "display.h"
#include "wifi_manager.h"
#include "websocket_handler.h"
#include "sensors.h"
#include "device_control.h"
#include "html_content.h"

// Define DHT global object that will be used across files
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    Serial.println("\nSmart Environment Hub - Starting up...");

    // Initialize LCD
    initDisplay();
    
    // Welcome splash screen
    displayWelcomeScreen();

    // Initialize sensors and outputs
    setupSensors();
    
    // Setup device control pins
    setupDeviceControl();

    // Blink LED to indicate startup
    for(int i = 0; i < 3; i++) {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }

    // Try to connect to WiFi or start Captive Portal
    setupWiFiConnection();
}

void loop() {
    // Handle DNS server for Captive Portal
    dnsServer.processNextRequest();

    // Handle WebSocket if connected
    if (isWiFiConnected) {
        webSocket.loop();

        // Send ping to keep connection alive
        if (millis() - lastPingTime >= PING_INTERVAL) {
            if (isWsConnected) {
                webSocket.sendTXT("ping");
            }
            lastPingTime = millis();
        }
    }

    // Check motion sensor and handle motion events
    checkMotion();

    // Regular LCD updates
    if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
        updateLCD();
        lastLCDUpdate = millis();
    }

    // Rotate display pages in normal operation
    if (currentLcdState == NORMAL_OPERATION && millis() - lastLCDUpdate >= DISPLAY_PAGE_INTERVAL) {
        currentDisplayPage = (currentDisplayPage + 1) % 2;  // Toggle between 0 and 1
        updateLCD();
        lastLCDUpdate = millis();
    }

    // Loading animation updates (faster than LCD)
    if (millis() - lastAnimationUpdate >= ANIMATION_INTERVAL &&
        (currentLcdState == CONNECTING_WIFI || currentLcdState == STARTING)) {
        displayLoadingAnimation();
        lastAnimationUpdate = millis();
    }

    // Send data to server
    if (isWiFiConnected && isWsConnected && millis() - lastDataSend >= DATA_SEND_INTERVAL) {
        sendDataToServer();
        lastDataSend = millis();
    }

    // Update device status
    if (isWiFiConnected && isWsConnected && millis() - lastDeviceUpdate >= DEVICE_UPDATE_INTERVAL) {
        updateDeviceStatus();
        lastDeviceUpdate = millis();
    }

    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED && isWiFiConnected) {
        isWiFiConnected = false;
        isWsConnected = false;
        isApiConnected = false;
        currentLcdState = CONNECTING_WIFI;
        updateLCD();

        // Try to reconnect before going back to AP mode
        WiFi.reconnect();
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            displayLoadingAnimation();
            delay(500);
            attempts++;
        }

        if (WiFi.status() != WL_CONNECTED) {
            setupCaptivePortal();  // Restart captive portal if reconnection fails
        } else {
            isWiFiConnected = true;
            currentLcdState = NORMAL_OPERATION;
            setupWebSocket();  // Reconnect WebSocket
        }
    }

    // Small delay to prevent CPU hogging
    delay(10);
}