#include "websocket_handler.h"
#include "config.h"
#include "display.h"
#include "device_control.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <DHT.h>

WebSocketsClient webSocket;

// Forward declaration
extern DHT dht;

void setupWebSocket() {
    // Connect to WebSocket server
    webSocket.begin(WS_SERVER, WS_PORT, WS_URL);

    // Set WebSocket event handler
    webSocket.onEvent(webSocketEvent);

    // Set reconnect interval to 5 seconds
    webSocket.setReconnectInterval(5000);

    Serial.println("WebSocket connection established");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("WebSocket disconnected!");
            isWsConnected = false;
            isApiConnected = false;
            break;

        case WStype_CONNECTED:
            Serial.println("WebSocket connected!");
            isWsConnected = true;
            isApiConnected = true;

            // Send initial data after connection established
            sendDataToServer();
            break;

        case WStype_TEXT: {
            Serial.printf("WebSocket message received: %s\n", payload);

            // Process received message
            JsonDocument responseDoc;
            DeserializationError error = deserializeJson(responseDoc, (char*)payload);

            if (!error) {
                // Check for device control commands
                if (responseDoc["device_control"].is<JsonObject>()) {
                    handleDeviceControl(responseDoc);
                }
                // Check for WiFi update commands
                else if (responseDoc["new_wifi"].is<JsonObject>()) {
                    String newSSID = responseDoc["new_wifi"]["ssid"];
                    String newPassword = responseDoc["new_wifi"]["password"];

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

                    currentLcdState = CONNECTING_WIFI;
                    updateLCD();

                    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
                        displayLoadingAnimation();
                        delay(250);
                        attempts++;
                    }

                    if (WiFi.status() == WL_CONNECTED) {
                        isWiFiConnected = true;
                        currentLcdState = NORMAL_OPERATION;
                        setupWebSocket();  // Reconnect WebSocket with new WiFi
                    } else {
                        // Import from wifi_manager.h
                        extern void setupCaptivePortal();
                        setupCaptivePortal(); // If new network fails, restart captive portal
                    }
                }
            }
            break;
        }

        case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            // Handle other WebSocket events if needed
            break;
    }
}

void sendDataToServer() {
    if (!isWiFiConnected || !isWsConnected) return;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        currentLcdState = SENSOR_ERROR;
        updateLCD();
        delay(2000);  // Show error briefly
        currentLcdState = NORMAL_OPERATION;
        return;
    }

    // Create JSON payload
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["motion"] = motionDetected;
    doc["wifi_strength"] = WiFi.RSSI();
    doc["device_id"] = WiFi.macAddress();
    doc["fan_speed"] = fanSpeed;
    doc["light1"] = light1Status;
    doc["light2"] = light2Status;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Send data through WebSocket
    webSocket.sendTXT(jsonPayload);
    isApiConnected = true;  // Consider API connected as long as WebSocket is connected
}

void updateDeviceStatus() {
    if (!isWiFiConnected || !isWsConnected) return;

    JsonDocument doc;
    doc["device_status"]["fan"] = fanSpeed;
    doc["device_status"]["light1"] = light1Status;
    doc["device_status"]["light2"] = light2Status;
    doc["device_id"] = WiFi.macAddress();

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Send status update
    webSocket.sendTXT(jsonPayload);
}