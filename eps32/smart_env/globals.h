/*
 * Global definitions and variables for Smart Environment Monitoring System
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <LiquidCrystal_I2C.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

// LCD display states
enum LcdState {
    STARTING,
    CONNECTING_WIFI,
    AP_MODE,
    NORMAL_OPERATION,
    API_ERROR,
    SENSOR_ERROR
    };

// Pin definitions
#define DHTPIN 15       // DHT temperature/humidity sensor
#define DHTTYPE DHT11
#define PIRPIN 18       // Motion sensor
#define LED 17          // Status LED
#define LCD_ADDR 0x27   // I2C address for LCD

// Configuration
#define AP_SSID "Smart Environment"
#define AP_PASSWORD ""
#define API_ENDPOINT "http://abc.xyz/data"

// Timing constants
#define MOTION_TIMEOUT 5000      // 5 seconds for motion LED
#define DATA_SEND_INTERVAL 5000  // 5 seconds between API updates
#define LCD_UPDATE_INTERVAL 1000 // 1 second between LCD updates
#define ANIMATION_INTERVAL 250   // 250ms between animation frames

// Global variables - will be defined in globals.cpp
extern LiquidCrystal_I2C LCD;
extern DNSServer dnsServer;
extern AsyncWebServer server;
extern bool isWiFiConnected;
extern bool motionDetected;
extern bool isApiConnected;
extern unsigned long lastMotionTime;
extern unsigned long lastDataSend;
extern unsigned long lastLCDUpdate;
extern unsigned long lastAnimationUpdate;
extern uint8_t animationFrame;
extern LcdState currentLcdState;

// IP Addresses for captive portal
extern const IPAddress localIP;
extern const IPAddress gatewayIP;
extern const IPAddress subnetMask;

// Portal HTML string
extern const char portal_html[];

#endif // GLOBALS_H
