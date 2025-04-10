/*
 * Display handling implementation for Smart Environment Monitoring System
 */

#include "display.h"

// WiFi connected icon
byte wifiIcon[8] = {
    0b00000, 0b00000, 0b00100, 0b01110, 0b10101, 0b00100, 0b00000, 0b00000
};

// AP mode icon
byte apIcon[8] = {
    0b00000, 0b01110, 0b10001, 0b10101, 0b10001, 0b01110, 0b00000, 0b00000
};

// Temperature icon
byte tempIcon[8] = {
    0b00100, 0b01010, 0b01010, 0b01010, 0b01110, 0b11111, 0b11111, 0b01110
};

// Humidity icon
byte humidityIcon[8] = {
    0b00100, 0b00100, 0b01010, 0b01010, 0b10001, 0b10001, 0b10001, 0b01110
};

// Motion icon
byte motionIcon[8] = {
    0b00000, 0b00100, 0b01110, 0b11111, 0b00100, 0b00100, 0b00000, 0b00000
};

// Loading animation frames (3 frames)
byte loadingIcon1[8] = {
    0b00000, 0b00000, 0b00000, 0b11100, 0b11100, 0b00000, 0b00000, 0b00000
};

byte loadingIcon2[8] = {
    0b00000, 0b00000, 0b00000, 0b00111, 0b00111, 0b00000, 0b00000, 0b00000
};

byte loadingIcon3[8] = {
    0b00000, 0b00000, 0b11100, 0b00000, 0b00000, 0b00111, 0b00000, 0b00000
};

void initDisplay() {
    Serial.println("Initializing display...");
    LCD.init();
    LCD.backlight();
    LCD.createChar(0, wifiIcon);
    LCD.createChar(1, apIcon);
    LCD.createChar(2, tempIcon);
    LCD.createChar(3, humidityIcon);
    LCD.createChar(4, motionIcon);
    LCD.createChar(5, loadingIcon1);
    LCD.createChar(6, loadingIcon2);
    LCD.createChar(7, loadingIcon3);
    Serial.println("Display initialized successfully");
}

void displayWelcomeScreen() {
    Serial.println("Displaying welcome screen");
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print(" Smart Home Hub ");
    LCD.setCursor(0, 1);
    LCD.print(" Initializing... ");
    delay(1500);
}

void displayLoadingAnimation() {
    animationFrame = (animationFrame + 1) % 3;
    
    Serial.printf("Animation frame: %d\n", animationFrame);
    
    switch(animationFrame) {
        case 0:
            LCD.setCursor(15, 1);
            LCD.write(5);  // Loading icon 1
            break;
        case 1:
            LCD.setCursor(15, 1);
            LCD.write(6);  // Loading icon 2
            break;
        case 2:
            LCD.setCursor(15, 1);
            LCD.write(7);  // Loading icon 3
            break;
    }
}

void updateLCD(LcdState state) {
    Serial.printf("Updating LCD with state: %d\n", state);
    
    switch(state) {
        case STARTING:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print(" Smart Home Hub ");
            LCD.setCursor(0, 1);
            LCD.print(" Initializing... ");
            break;

        case CONNECTING_WIFI:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("Connecting WiFi");
            LCD.setCursor(0, 1);
            LCD.print("Please wait");
            displayLoadingAnimation();
            break;

        case AP_MODE:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.write(1);  // AP icon
            LCD.print(" WiFi Setup Mode");
            LCD.setCursor(0, 1);
            LCD.print("Connect: ");
            LCD.print(AP_SSID);
            break;

        case NORMAL_OPERATION:
            // This should be handled by displaySensorData()
            Serial.println("Warning: NORMAL_OPERATION called directly to updateLCD");
            break;

        case API_ERROR:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("API Connection");
            LCD.setCursor(0, 1);
            LCD.print("Failed!");
            break;

        case SENSOR_ERROR:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("Sensor Error");
            LCD.setCursor(0, 1);
            LCD.print("Check DHT11");
            break;
    }
}

void displaySensorData(bool isWiFiConnected, bool motionDetected, bool isApiConnected,
                       float temperature, float humidity) {
    Serial.println("Displaying sensor data");
    Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
    Serial.printf("WiFi: %s, Motion: %s, API: %s\n", 
                 isWiFiConnected ? "Connected" : "Disconnected",
                 motionDetected ? "Detected" : "Not detected",
                 isApiConnected ? "Connected" : "Disconnected");
    
    LCD.clear();

    // First row: WiFi status and temperature
    if (isWiFiConnected) {
        LCD.write(0);  // WiFi icon
    } else {
        LCD.write(1);  // AP icon
    }

    LCD.setCursor(2, 0);
    LCD.write(2);  // Temperature icon

    if (!isnan(temperature)) {
        LCD.print(temperature, 1);
        LCD.print("C");
    } else {
        LCD.print("--.-C");
    }

    // Add motion indicator if detected
    if (motionDetected) {
        LCD.setCursor(14, 0);
        LCD.write(4);  // Motion icon
    }

    // Second row: Humidity and connection status
    LCD.setCursor(2, 1);
    LCD.write(3);  // Humidity icon

    if (!isnan(humidity)) {
        LCD.print(humidity, 1);
        LCD.print("%");
    } else {
        LCD.print("--.-");
    }

    // API status indicator
    LCD.setCursor(14, 1);
    if (isApiConnected) {
        LCD.print("A");  // API connected
    } else {
        LCD.print("X");  // API disconnected
    }
}
