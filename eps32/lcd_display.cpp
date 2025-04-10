/*
 * File: lcd_display.cpp
 * Description: Implementation of LCD display control functions
 */

#include "lcd_display.h"

// Declare LCD I2C 16x2 screen (0x27 is the common I2C address for LCD)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create custom characters for LCD
uint8_t wifiIcon[8] = {0x00, 0x00, 0x0E, 0x11, 0x04, 0x0A, 0x00, 0x00};
uint8_t tempIcon[8] = {0x04, 0x0A, 0x0A, 0x0A, 0x0A, 0x0E, 0x0E, 0x04};
uint8_t humidIcon[8] = {0x04, 0x04, 0x0A, 0x0A, 0x11, 0x11, 0x11, 0x0E};
uint8_t motionIcon[8] = {0x04, 0x0E, 0x15, 0x04, 0x04, 0x04, 0x04, 0x00};

void setupLCD() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();  // Turn on backlight
  
  // Create custom icons
  lcd.createChar(WIFI_ICON, wifiIcon);
  lcd.createChar(TEMP_ICON, tempIcon);
  lcd.createChar(HUMID_ICON, humidIcon);
  lcd.createChar(MOTION_ICON, motionIcon);
  
  Serial.println("LCD display initialized");
}

void displayBootScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Home System");
  lcd.setCursor(0, 1);
  lcd.print("Booting...");
  delay(2000);
}

void displayWiFiConnecting(const String& message, const String& ssid) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  
  if (ssid.length() > 0) {
    lcd.setCursor(0, 1);
    // If SSID is too long, truncate it to fit the screen
    if (ssid.length() > 16) {
      lcd.print(ssid.substring(0, 13) + "...");
    } else {
      lcd.print(ssid);
    }
  }
}

void displayAPMode(const String& apName) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AP: " + (apName.length() > 12 ? apName.substring(0, 9) + "..." : apName));
  lcd.setCursor(0, 1);
  lcd.print("IP:");
  // IP will be displayed by the WiFi module after calling this function
}

void updateDisplay(float temperature, float humidity, bool motion, bool wifiStatus) {
  lcd.clear();
  
  // Row 1: WiFi icon and temperature
  lcd.setCursor(0, 0);
  if (wifiStatus) {
    lcd.write(WIFI_ICON); // Display WiFi icon
  } else {
    lcd.print("X");       // No WiFi connection
  }
  
  lcd.setCursor(2, 0);
  lcd.write(TEMP_ICON);  // Temperature icon
  lcd.print(" ");
  lcd.print(temperature, 1);  // Display with 1 decimal place
  lcd.print((char)223);       // Degree symbol (°)
  lcd.print("C");
  
  // If motion is detected, display motion icon in the right corner
  if (motion) {
    lcd.setCursor(15, 0);
    lcd.write(MOTION_ICON);
  }
  
  // Row 2: Humidity
  lcd.setCursor(0, 1);
  lcd.write(HUMID_ICON);  // Humidity icon
  lcd.print(" ");
  lcd.print(humidity, 1);  // Display with 1 decimal place
  lcd.print("%");
}

void displayError(const String& error) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR!");
  lcd.setCursor(0, 1);
  
  // If error message is too long, truncate it to fit the screen
  if (error.length() > 16) {
    lcd.print(error.substring(0, 13) + "...");
  } else {
    lcd.print(error);
  }
}