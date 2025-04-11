#include "display.h"
#include "custom_characters.h"
#include <DHT.h>

// External references to objects and variables needed here
extern DHT dht;
LiquidCrystal_I2C LCD(LCD_ADDR, 16, 2);

void initDisplay() {
    // Initialize LCD
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

    // Create device control icons (overwrite some of the loading icons)
    LCD.createChar(5, fanIcon);
    LCD.createChar(6, lightIcon);
}

void displayWelcomeScreen() {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print(" Smart Home Hub ");
    LCD.setCursor(0, 1);
    LCD.print(" Initializing... ");
    delay(1500);
}

void displayLoadingAnimation() {
    // Create a temporary buffer for loading animation
    static byte loadingIconTemp[8];

    // Copy the loading icon data
    for (int i = 0; i < 8; i++) {
        switch(animationFrame) {
            case 0:
                loadingIconTemp[i] = loadingIcon1[i];
                break;
            case 1:
                loadingIconTemp[i] = loadingIcon2[i];
                break;
            case 2:
                loadingIconTemp[i] = loadingIcon3[i];
                break;
        }
    }

    // Create the temporary character
    LCD.createChar(7, loadingIconTemp);

    // Display the animation
    LCD.setCursor(15, 1);
    LCD.write(7);

    // Advance animation frame
    animationFrame = (animationFrame + 1) % 3;
}

void updateLCD() {
    switch(currentLcdState) {
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
            LCD.print(apSSID);
            break;

        case NORMAL_OPERATION:
            // Rotate between sensor data and device status
            if (currentDisplayPage == 0) {
                displaySensorData();
            } else {
                displayDeviceStatus();
            }
            break;

        case API_ERROR:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("WS Connection");
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

void displaySensorData() {
    LCD.clear();

    // First row: WiFi status and temperature
    if (isWiFiConnected) {
        LCD.write(0);  // WiFi icon
    } else {
        LCD.write(1);  // AP icon
    }

    LCD.setCursor(2, 0);
    LCD.write(2);  // Temperature icon

    float temp = dht.readTemperature();
    if (!isnan(temp)) {
        LCD.print(temp, 1);
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

    float hum = dht.readHumidity();
    if (!isnan(hum)) {
        LCD.print(hum, 1);
        LCD.print("%");
    } else {
        LCD.print("--.-");
    }

    // API status indicator
    LCD.setCursor(14, 1);
    if (isWsConnected) {
        LCD.print("W");  // WebSocket connected
    } else {
        LCD.print("X");  // WebSocket disconnected
    }
}

void displayDeviceStatus() {
    LCD.clear();

    // First row: Fan status
    LCD.setCursor(0, 0);
    LCD.write(5);  // Fan icon
    LCD.print(" Fan: ");

    // Display fan speed as percentage
    int fanPercent = map(fanSpeed, 0, 255, 0, 100);
    LCD.print(fanPercent);
    LCD.print("%");

    // Second row: Light statuses
    LCD.setCursor(0, 1);
    LCD.write(6);  // Light icon
    LCD.print(" L1:");
    LCD.print(light1Status ? "ON" : "OFF");

    LCD.setCursor(9, 1);
    LCD.print("L2:");
    LCD.print(light2Status ? "ON" : "OFF");
}