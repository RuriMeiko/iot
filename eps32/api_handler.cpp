/*
 * File: api_handler.cpp
 * Description: Implementation of API interface functions
 */

#include "api_handler.h"
#include "sensors.h"
#include "wifi_manager.h"

// API URL
const char* apiEndpoint = "http://abc.xyz/api/data";

// Time between API sends (10 seconds)
const unsigned long API_SEND_INTERVAL = 10000;

// Timer counter for API
unsigned long lastApiSendTime = 0;

void setupAPI() {
  // Initialize API handler
  Serial.println("API handler initialized");
  Serial.print("API Endpoint: ");
  Serial.println(apiEndpoint);
}

void handleAPI() {
  // Check if it's time to send data to the API
  unsigned long currentTime = millis();
  
  if (currentTime - lastApiSendTime >= API_SEND_INTERVAL) {
    lastApiSendTime = currentTime;
    
    // Get data from sensors
    float temperature = getTemperature();
    float humidity = getHumidity();
    bool motion = isMotionDetected();
    
    // Send data to API
    if (sendDataToAPI(temperature, humidity, motion)) {
      Serial.println("API data sent successfully");
    } else {
      Serial.println("Unable to send data to API");
    }
  }
}

bool sendDataToAPI(float temperature, float humidity, bool motion) {
  // Check WiFi connection
  if (!isWiFiConnected()) {
    Serial.println("Cannot send data: no WiFi connection");
    return false;
  }
  
  // Create JSON to send to API
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["motion"] = motion;
  jsonDoc["wifi_status"] = getWiFiStatus();
  jsonDoc["ssid"] = getWiFiSSID();
  jsonDoc["ip"] = WiFi.localIP().toString();
  
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  
  // Send data to API
  HTTPClient http;
  http.begin(apiEndpoint);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("API Response: ");
    Serial.println(response);
    
    // Process response from API
    processAPIResponse(response);
    http.end();
    return true;
  } else {
    Serial.print("HTTP request error: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

void processAPIResponse(const String& response) {
  // Parse JSON response from API
  StaticJsonDocument<200> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, response);
  
  if (error) {
    Serial.print("JSON parsing error: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Check if there are commands to change WiFi
  if (jsonDoc.containsKey("wifi_ssid") && jsonDoc.containsKey("wifi_password")) {
    String newSSID = jsonDoc["wifi_ssid"].as<String>();
    String newPassword = jsonDoc["wifi_password"].as<String>();
    
    Serial.println("Received new WiFi configuration from API");
    Serial.print("New SSID: ");
    Serial.println(newSSID);
    
    // Change WiFi info if different from current info
    if (newSSID != getWiFiSSID()) {
      Serial.println("Changing WiFi connection...");
      
      // Change WiFi credentials
      changeWiFiCredentials(newSSID, newPassword);
    }
  }
  
  // Can process other commands from API here
  // Examples: control LCD display, change data sending frequency, etc.
}