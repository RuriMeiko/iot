/*
 * File: api_handler.cpp
 * Mu00f4 tu1ea3: Triu1ec3n khai cu00e1c hu00e0m giao tiu1ebfp API
 */

#include "api_handler.h"
#include "sensors.h"
#include "wifi_manager.h"

// URL API
const char* apiEndpoint = "http://abc.xyz/api/data";

// Thu1eddi gian giu1eefa cu00e1c lu1ea7n gu1eedi API (10 giu00e2y)
const unsigned long API_SEND_INTERVAL = 10000;

// Bu1ed9 u0111u1ebfm thu1eddi gian cho API
unsigned long lastApiSendTime = 0;

void setupAPI() {
  // Khu1edfi tu1ea1o API handler
  Serial.println("API handler u0111u00e3 u0111u01b0u1ee3c khu1edfi tu1ea1o");
  Serial.print("API Endpoint: ");
  Serial.println(apiEndpoint);
}

void handleAPI() {
  // Kiu1ec3m tra xem u0111u00e3 u0111u1ebfn lu00fac gu1eedi du1eef liu1ec7u lu00ean API chu01b0a
  unsigned long currentTime = millis();
  
  if (currentTime - lastApiSendTime >= API_SEND_INTERVAL) {
    lastApiSendTime = currentTime;
    
    // Lu1ea5y du1eef liu1ec7u tu1eeb cu00e1c cu1ea3m biu1ebfn
    float temperature = getTemperature();
    float humidity = getHumidity();
    bool motion = isMotionDetected();
    
    // Gu1eedi du1eef liu1ec7u lu00ean API
    if (sendDataToAPI(temperature, humidity, motion)) {
      Serial.println("Gu1eedi du1eef liu1ec7u API thu00e0nh cu00f4ng");
    } else {
      Serial.println("Khu00f4ng thu1ec3 gu1eedi du1eef liu1ec7u u0111u1ebfn API");
    }
  }
}

bool sendDataToAPI(float temperature, float humidity, bool motion) {
  // Kiu1ec3m tra ku1ebft nu1ed1i WiFi
  if (!isWiFiConnected()) {
    Serial.println("Khu00f4ng thu1ec3 gu1eedi du1eef liu1ec7u: khu00f4ng cu00f3 ku1ebft nu1ed1i WiFi");
    return false;
  }
  
  // Tu1ea1o JSON u0111u1ec3 gu1eedi lu00ean API
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["motion"] = motion;
  jsonDoc["wifi_status"] = getWiFiStatus();
  jsonDoc["ssid"] = getWiFiSSID();
  jsonDoc["ip"] = WiFi.localIP().toString();
  
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  
  // Gu1eedi du1eef liu1ec7u lu00ean API
  HTTPClient http;
  http.begin(apiEndpoint);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("API Response: ");
    Serial.println(response);
    
    // Xu1eed lu00fd phu1ea3n hu1ed3i tu1eeb API
    processAPIResponse(response);
    http.end();
    return true;
  } else {
    Serial.print("Lu1ed7i gu1eedi HTTP request: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

void processAPIResponse(const String& response) {
  // Phu00e2n tu00edch phu1ea3n hu1ed3i JSON tu1eeb API
  StaticJsonDocument<200> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, response);
  
  if (error) {
    Serial.print("Lu1ed7i phu00e2n tu00edch JSON: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Kiu1ec3m tra xem cu00f3 lu1ec7nh thay u0111u1ed5i WiFi khu00f4ng
  if (jsonDoc.containsKey("wifi_ssid") && jsonDoc.containsKey("wifi_password")) {
    String newSSID = jsonDoc["wifi_ssid"].as<String>();
    String newPassword = jsonDoc["wifi_password"].as<String>();
    
    Serial.println("Nhu1eadn u0111u01b0u1ee3c cu1ea5u hu00ecnh WiFi mu1edbi tu1eeb API");
    Serial.print("SSID mu1edbi: ");
    Serial.println(newSSID);
    
    // Thay u0111u1ed5i thu00f4ng tin WiFi nu1ebfu khu00e1c vu1edbi thu00f4ng tin hiu1ec7n tu1ea1i
    if (newSSID != getWiFiSSID()) {
      Serial.println("Thay u0111u1ed5i ku1ebft nu1ed1i WiFi...");
      
      // Thay u0111u1ed5i thu00f4ng tin WiFi
      changeWiFiCredentials(newSSID, newPassword);
    }
  }
  
  // Cu00f3 thu1ec3 xu1eed lu00fd cu00e1c lu1ec7nh khu00e1c tu1eeb API tu1ea1i u0111u00e2y
  // Vu00ed du1ee5: u0111iu1ec1u khiu1ec3n hiu1ec7n thu1ecb LCD, thay u0111u1ed5i tu1ea7n su1ea5t gu1eedi du1eef liu1ec7u, v.v.
}