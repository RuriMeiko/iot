/*
 * File: smart_home.ino
 * Description: Main file of smart home management project
 * Author: AI Assistant
 */

// Include libraries and modules
#include "common.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "lcd_display.h"
#include "api_handler.h"

void setup() {
  // Initialize serial interface for debugging
  Serial.begin(115200);
  Serial.println("\n--- Smart home system booting ---");
  
  // Initialize LCD display
  setupLCD();
  
  // Show boot screen
  displayBootScreen();
  
  // Initialize sensors
  setupSensors();
  Serial.println("Sensors initialized");
  
  // Initialize WiFi connection
  setupWiFi();
  
  // Initialize API handler
  setupAPI();
  
  Serial.println("System initialization complete!");
}

void loop() {
  // Check WiFi connection status
  if (isWiFiConnected()) {
    // If connected, perform main functions
    
    // Read and process information from sensors
    handleSensors();
    
    // Update display information on LCD
    updateDisplay(getTemperature(), getHumidity(), isMotionDetected(), getWiFiStatus());
    
    // Send data to API and process response
    handleAPI();
  } else {
    // If not connected, manage Access Point mode
    handleAPMode();
  }
  
  // Prevent loop from running too fast
  delay(100);
}