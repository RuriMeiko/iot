/*
 * File: lcd_display.h
 * Description: Manage 16x2 LCD display, including configuration and display functions
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "common.h"

// Define custom characters for LCD
#define WIFI_ICON   0  // WiFi icon
#define TEMP_ICON   1  // Temperature icon
#define HUMID_ICON  2  // Humidity icon
#define MOTION_ICON 3  // Motion icon

// Global variable declaration
extern LiquidCrystal_I2C lcd;

// ======= FUNCTION DECLARATIONS =======

/**
 * Initialize LCD display and create custom characters
 */
void setupLCD();

/**
 * Display boot screen
 */
void displayBootScreen();

/**
 * Display information about the WiFi connection process
 * @param message Message to display
 * @param ssid    Name of the WiFi network connecting to (optional)
 */
void displayWiFiConnecting(const String& message, const String& ssid = "");

/**
 * Display information about AP mode
 * @param apName Name of the Access Point
 */
void displayAPMode(const String& apName);

/**
 * Update display information on LCD
 * @param temperature Temperature (°C)
 * @param humidity    Humidity (%)
 * @param motion      Motion sensor state
 * @param wifiStatus  WiFi connection status
 */
void updateDisplay(float temperature, float humidity, bool motion, bool wifiStatus);

/**
 * Display error message
 * @param error Error message to display
 */
void displayError(const String& error);

#endif // LCD_DISPLAY_H