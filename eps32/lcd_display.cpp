/*
 * File: lcd_display.cpp
 * Mu00f4 tu1ea3: Cu00e0i u0111u1eb7t cu00e1c hu00e0m u0111iu1ec1u khiu1ec3n mu00e0n hu00ecnh LCD
 */

#include "lcd_display.h"

// Khai bu00e1o mu00e0n hu00ecnh LCD I2C 16x2 (0x27 lu00e0 u0111u1ecba chu1ec9 I2C thu00f4ng du1ee5ng cho LCD)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Tu1ea1o custom characters cho LCD
uint8_t wifiIcon[8] = {0x00, 0x00, 0x0E, 0x11, 0x04, 0x0A, 0x00, 0x00};
uint8_t tempIcon[8] = {0x04, 0x0A, 0x0A, 0x0A, 0x0A, 0x0E, 0x0E, 0x04};
uint8_t humidIcon[8] = {0x04, 0x04, 0x0A, 0x0A, 0x11, 0x11, 0x11, 0x0E};
uint8_t motionIcon[8] = {0x04, 0x0E, 0x15, 0x04, 0x04, 0x04, 0x04, 0x00};

void setupLCD() {
  // Khu1edfi tu1ea1o LCD
  lcd.init();
  lcd.backlight();  // Bu1eadt u0111u00e8n nu1ec1n
  
  // Tu1ea1o cu00e1c biu1ec3u tu01b0u1ee3ng tu00f9y chu1ec9nh
  lcd.createChar(WIFI_ICON, wifiIcon);
  lcd.createChar(TEMP_ICON, tempIcon);
  lcd.createChar(HUMID_ICON, humidIcon);
  lcd.createChar(MOTION_ICON, motionIcon);
  
  Serial.println("Mu00e0n hu00ecnh LCD u0111u00e3 u0111u01b0u1ee3c khu1edfi tu1ea1o");
}

void displayBootScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Home System");
  lcd.setCursor(0, 1);
  lcd.print("Dang khoi dong...");
  delay(2000);
}

void displayWiFiConnecting(const String& message, const String& ssid) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  
  if (ssid.length() > 0) {
    lcd.setCursor(0, 1);
    // Nu1ebfu ssid quu00e1 du00e0i, cu1eaft bu1edbt u0111u1ec3 vu1eeba mu00e0n hu00ecnh
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
  // IP su1ebd u0111u01b0u1ee3c hiu1ec3n thu1ecb bu1edfi WiFi module sau khi gu1ecdi hu00e0m nu00e0y
}

void updateDisplay(float temperature, float humidity, bool motion, bool wifiStatus) {
  lcd.clear();
  
  // Du00f2ng 1: Biu1ec3u tu01b0u1ee3ng WiFi vu00e0 nhiu1ec7t u0111u1ed9
  lcd.setCursor(0, 0);
  if (wifiStatus) {
    lcd.write(WIFI_ICON); // Hiu1ec3n thu1ecb biu1ec3u tu01b0u1ee3ng WiFi
  } else {
    lcd.print("X");       // Khu00f4ng cu00f3 ku1ebft nu1ed1i WiFi
  }
  
  lcd.setCursor(2, 0);
  lcd.write(TEMP_ICON);  // Biu1ec3u tu01b0u1ee3ng nhiu1ec7t u0111u1ed9
  lcd.print(" ");
  lcd.print(temperature, 1);  // Hiu1ec3n thu1ecb vu1edbi 1 su1ed1 thu1eadp phu00e2n
  lcd.print((char)223);       // Ku00fd tu1ef1 u0111u1ed9 (u00b0)
  lcd.print("C");
  
  // Nu1ebfu cu00f3 chuyu1ec3n u0111u1ed9ng, hiu1ec3n thu1ecb biu1ec3u tu01b0u1ee3ng u1edf gu00f3c phu1ea3i
  if (motion) {
    lcd.setCursor(15, 0);
    lcd.write(MOTION_ICON);
  }
  
  // Du00f2ng 2: u0110u1ed9 u1ea9m
  lcd.setCursor(0, 1);
  lcd.write(HUMID_ICON);  // Biu1ec3u tu01b0u1ee3ng u0111u1ed9 u1ea9m
  lcd.print(" ");
  lcd.print(humidity, 1);  // Hiu1ec3n thu1ecb vu1edbi 1 su1ed1 thu1eadp phu00e2n
  lcd.print("%");
}

void displayError(const String& error) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LOI!");
  lcd.setCursor(0, 1);
  
  // Nu1ebfu thu00f4ng bu00e1o lu1ed7i quu00e1 du00e0i, cu1eaft bu1edbt u0111u1ec3 vu1eeba mu00e0n hu00ecnh
  if (error.length() > 16) {
    lcd.print(error.substring(0, 13) + "...");
  } else {
    lcd.print(error);
  }
}

LiquidCrystal_I2C& getLCD() {
  return lcd;
}