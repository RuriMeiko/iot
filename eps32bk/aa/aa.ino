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

    // Handle WebSocket if connected and if we want to use WebSocket
    if (isWiFiConnected && false) { // Set to true if you want to enable WebSocket
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

    // Send data to server - disabled as WebSocket is disabled
    if (isWiFiConnected && isWsConnected && false && millis() - lastDataSend >= DATA_SEND_INTERVAL) {
        sendDataToServer();
        lastDataSend = millis();
    }

    // Update device status - disabled as WebSocket is disabled
    if (isWiFiConnected && isWsConnected && false && millis() - lastDeviceUpdate >= DEVICE_UPDATE_INTERVAL) {
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
      static unsigned long lastReconnectAttempt = 0;
      static int reconnectAttempts = 0;
      if (reconnectAttempts < 10 && millis() - lastReconnectAttempt >= 500) {
          displayLoadingAnimation();
          WiFi.reconnect();
          reconnectAttempts++;
          lastReconnectAttempt = millis();
      } else if (reconnectAttempts >= 10) {
          setupCaptivePortal();
          reconnectAttempts = 0;
      } else if (WiFi.status() == WL_CONNECTED) {
          isWiFiConnected = true;
          currentLcdState = NORMAL_OPERATION;
          reconnectAttempts = 0;
          setupWebSocket();
      }
  }

    // Small delay to prevent CPU hogging
    delay(10);
}
