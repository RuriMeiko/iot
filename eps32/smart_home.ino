/*
 * File: smart_home.ino
 * Mu00f4 tu1ea3: File chu00ednh cu1ee7a du1ef1 u00e1n quu1ea3n lu00fd nhu00e0 thu00f4ng minh
 * Tu00e1c giu1ea3: AI Assistant
 */

// Include cu00e1c thu01b0 viu1ec7n vu00e0 module
#include "common.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "lcd_display.h"
#include "api_handler.h"

void setup() {
  // Khu1edfi tu1ea1o giao tiu1ebfp serial u0111u1ec3 debug
  Serial.begin(115200);
  Serial.println("\n--- Hu1ec7 thu1ed1ng nhu00e0 thu00f4ng minh u0111ang khu1edfi u0111u1ed9ng ---");
  
  // Khu1edfi tu1ea1o mu00e0n hu00ecnh LCD
  setupLCD();
  
  // Hiu1ec3n thu1ecb thu00f4ng bu00e1o khu1edfi u0111u1ed9ng
  displayBootScreen();
  
  // Khu1edfi tu1ea1o cu00e1c cu1ea3m biu1ebfn
  setupSensors();
  Serial.println("Cu1ea3m biu1ebfn u0111u00e3 khu1edfi tu1ea1o xong");
  
  // Khu1edfi tu1ea1o ku1ebft nu1ed1i WiFi
  setupWiFi();
  
  // Khu1edfi tu1ea1o API handler
  setupAPI();
  
  Serial.println("Hu1ec7 thu1ed1ng khu1edfi tu1ea1o hou00e0n tu1ea5t!");
}

void loop() {
  // Kiu1ec3m tra tru1ea1ng thu00e1i ku1ebft nu1ed1i WiFi
  if (isWiFiConnected()) {
    // Nu1ebfu u0111u00e3 ku1ebft nu1ed1i, thu1ef1c hiu1ec7n cu00e1c chu1ee9c nu0103ng chu00ednh
    
    // u0110u1ecdc vu00e0 xu1eed lu00fd thu00f4ng tin tu1eeb cu00e1c cu1ea3m biu1ebfn
    handleSensors();
    
    // Cu1eadp nhu1eadt thu00f4ng tin hiu1ec3n thu1ecb tru00ean LCD
    updateDisplay(getTemperature(), getHumidity(), isMotionDetected(), getWiFiStatus());
    
    // Gu1eedi du1eef liu1ec7u lu00ean API vu00e0 xu1eed lu00fd phu1ea3n hu1ed3i
    handleAPI();
  } else {
    // Nu1ebfu chu01b0a ku1ebft nu1ed1i, quu1ea3n lu00fd chu1ebf u0111u1ed9 Access Point
    handleAPMode();
  }
  
  // Tru00e1nh loop chu1ea1y quu00e1 nhanh
  delay(100);
}