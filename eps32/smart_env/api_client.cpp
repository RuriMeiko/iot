/*
 * API client implementation for Smart Environment Monitoring System
 */

#include "api_client.h"
#include "wifi_manager.h"
#include "display.h"


void sendDataToServer(float temperature, float humidity, bool motionDetected) {
    if (!isWiFiConnected) {
        Serial.println("Cannot send data: WiFi not connected");
        return;
    }

    Serial.println("Sending data to server...");
    
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("ERROR: Invalid sensor data (NaN values)");
        updateLCD(SENSOR_ERROR);
        delay(2000);  // Show error briefly
        updateLCD(NORMAL_OPERATION);
        return;
    }

    HTTPClient http;
    http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    
    Serial.printf("Using API endpoint: %s\n", API_ENDPOINT);

    // Create JSON payload using ArduinoJson 6.x syntax
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["motion"] = motionDetected;
    doc["wifi_strength"] = WiFi.RSSI();
    doc["device_id"] = WiFi.macAddress();

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    Serial.printf("JSON payload: %s\n", jsonPayload.c_str());

    int httpCode = http.POST(jsonPayload);
    Serial.printf("HTTP response code: %d\n", httpCode);

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
        isApiConnected = true;
        String response = http.getString();
        Serial.printf("Response: %s\n", response.c_str());

        // Process possible server commands
        JsonDocument responseDoc;
        DeserializationError error = deserializeJson(responseDoc, response);

        if (!error && responseDoc.containsKey("new_wifi")) {
            String newSSID = responseDoc["new_wifi"]["ssid"];
            String newPassword = responseDoc["new_wifi"]["password"];

            Serial.println("Received new WiFi configuration from server");
            Serial.printf("New SSID: %s\n", newSSID.c_str());

            // Display WiFi update notification
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("New WiFi Config:");
            LCD.setCursor(0, 1);
            LCD.print(newSSID);
            delay(2000);

            // Connect to new network
            WiFi.begin(newSSID.c_str(), newPassword.c_str());
            int attempts = 0;

            updateLCD(CONNECTING_WIFI);

            while (WiFi.status() != WL_CONNECTED && attempts < 20) {
                displayLoadingAnimation();
                delay(250);
                attempts++;
                Serial.printf("Connection attempt %d/20\n", attempts);
            }

            if (WiFi.status() == WL_CONNECTED) {
                isWiFiConnected = true;
                Serial.println("Successfully connected to new WiFi network");
                updateLCD(NORMAL_OPERATION);
            } else {
                Serial.println("Failed to connect to new WiFi network, restarting captive portal");
                setupCaptivePortal(); // If new network fails, restart captive portal
            }
        }
    } else {
        isApiConnected = false;
        Serial.println("API connection failed");
        updateLCD(API_ERROR);
        delay(2000);  // Show error briefly
        updateLCD(NORMAL_OPERATION);
    }

    http.end();
    Serial.println("Data transmission completed");
}
