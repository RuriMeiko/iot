#include "LCDManager.h"

// Custom character definitions
const uint8_t WIFI_ICON[8] = {
    0b00000, 0b00000, 0b00100, 0b01110, 0b10101, 0b00100, 0b00000, 0b00000
};
const uint8_t AP_ICON[8] = {
    0b00000, 0b01110, 0b10001, 0b10101, 0b10001, 0b01110, 0b00000, 0b00000
};
const uint8_t TEMP_ICON[8] = {
    0b00100, 0b01010, 0b01010, 0b01010, 0b01110, 0b11111, 0b11111, 0b01110
};
const uint8_t HUMIDITY_ICON[8] = {
    0b00100, 0b00100, 0b01010, 0b01010, 0b10001, 0b10001, 0b10001, 0b01110
};
const uint8_t MOTION_ICON[8] = {
    0b00000, 0b00100, 0b01110, 0b11111, 0b00100, 0b00100, 0b00000, 0b00000
};
const uint8_t LOADING_ICON1[8] = {
    0b00000, 0b00000, 0b00000, 0b11100, 0b11100, 0b00000, 0b00000, 0b00000
};
const uint8_t LOADING_ICON2[8] = {
    0b00000, 0b00000, 0b00000, 0b00111, 0b00111, 0b00000, 0b00000, 0b00000
};
const uint8_t LOADING_ICON3[8] = {
    0b00000, 0b00000, 0b11100, 0b00000, 0b00000, 0b00111, 0b00000, 0b00000
};

LCDManager::LCDManager(uint8_t lcd_addr, uint8_t cols, uint8_t rows) : lcd(lcd_addr, cols, rows) {
    animationFrame = 0;
    lastAnimationUpdate = 0;
}

void LCDManager::init() {
    lcd.init();
    lcd.backlight();

    // Create custom characters
    lcd.createChar(0, (uint8_t*)WIFI_ICON);
    lcd.createChar(1, (uint8_t*)AP_ICON);
    lcd.createChar(2, (uint8_t*)TEMP_ICON);
    lcd.createChar(3, (uint8_t*)HUMIDITY_ICON);
    lcd.createChar(4, (uint8_t*)MOTION_ICON);
    lcd.createChar(5, (uint8_t*)LOADING_ICON1);
    lcd.createChar(6, (uint8_t*)LOADING_ICON2);
    lcd.createChar(7, (uint8_t*)LOADING_ICON3);
}

void LCDManager::displayWelcomeScreen() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Smart Home Hub ");
    lcd.setCursor(0, 1);
    lcd.print(" Initializing... ");
}

void LCDManager::displayLoadingAnimation() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastAnimationUpdate >= 250) { // Animation speed
        animationFrame = (animationFrame + 1) % 3;
        lastAnimationUpdate = currentMillis;

        lcd.setCursor(15, 1);
        lcd.write(5 + animationFrame); // Use the appropriate loading icon
    }
}

void LCDManager::updateDisplay(LcdState state, float temp, float humid, bool motion, bool wifiConnected, bool apiConnected) {
    switch(state) {
        case STARTING:
            displayWelcomeScreen();
            break;

        case CONNECTING_WIFI:
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Connecting WiFi");
            lcd.setCursor(0, 1);
            lcd.print("Please wait");
            displayLoadingAnimation();
            break;

        case AP_MODE:
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.write(1);  // AP icon
            lcd.print(" WiFi Setup Mode");
            lcd.setCursor(0, 1);
            lcd.print("Connect to setup");
            break;

        case NORMAL_OPERATION:
            lcd.clear();

            // First row: WiFi status and temperature
            if (wifiConnected) {
                lcd.write(0);  // WiFi icon
            } else {
                lcd.write(1);  // AP icon
            }

            lcd.setCursor(2, 0);
            lcd.write(2);  // Temperature icon

            if (!isnan(temp)) {
                lcd.print(temp, 1);
                lcd.print("C");
            } else {
                lcd.print("--.-C");
            }

            // Add motion indicator if detected
            if (motion) {
                lcd.setCursor(14, 0);
                lcd.write(4);  // Motion icon
            }

            // Second row: Humidity and connection status
            lcd.setCursor(2, 1);
            lcd.write(3);  // Humidity icon

            if (!isnan(humid)) {
                lcd.print(humid, 1);
                lcd.print("%");
            } else {
                lcd.print("--.-");
            }

            // API status indicator
            lcd.setCursor(14, 1);
            if (apiConnected) {
                lcd.print("A");  // API connected
            } else {
                lcd.print("X");  // API disconnected
            }
            break;

        case API_ERROR:
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("API Connection");
            lcd.setCursor(0, 1);
            lcd.print("Failed!");
            break;

        case SENSOR_ERROR:
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Sensor Error");
            lcd.setCursor(0, 1);
            lcd.print("Check DHT11");
            break;
    }
}

void LCDManager::showWiFiConnecting(String ssid) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to:");
    lcd.setCursor(0, 1);
    // Truncate SSID if too long
    if (ssid.length() > 16) {
        ssid = ssid.substring(0, 13) + "...";
    }
    lcd.print(ssid);
}

void LCDManager::showWiFiConnected(String ip) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(0);  // WiFi icon
    lcd.print(" Connected!");
    lcd.setCursor(0, 1);
    lcd.print(ip);
}

void LCDManager::showWiFiError() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connection");
    lcd.setCursor(0, 1);
    lcd.print("Failed!");
}

void LCDManager::showAPMode(String ssid) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(1);  // AP icon
    lcd.print(" AP: ");
    if (ssid.length() > 10) {
        ssid = ssid.substring(0, 7) + "...";
    }
    lcd.print(ssid);
    lcd.setCursor(0, 1);
    lcd.print("IP: 4.3.2.1");
}
