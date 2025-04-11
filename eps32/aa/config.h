#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========== PIN DEFINITIONS ==========
#define DHTPIN 14       // DHT temperature/humidity sensor
#define DHTTYPE DHT11
#define PIRPIN 18       // Motion sensor
#define LED 17          // Status LED
#define LCD_ADDR 0x27   // I2C address for LCD

// New device control pins
#define FAN_PIN 25      // Fan speed control (analog/PWM output)
#define LIGHT1_PIN 26   // Light 1 control (digital output)
#define LIGHT2_PIN 27   // Light 2 control (digital output)

// ========== CONFIGURATION ==========
extern const char* apSSID;
extern const char* apPassword;
extern const char* WS_SERVER;
extern const int WS_PORT;
extern const char* WS_URL;

// ========== GLOBAL VARIABLES ==========
extern bool isWiFiConnected;
extern bool motionDetected;
extern bool isApiConnected;
extern bool isWsConnected;
extern bool light1Status;
extern bool light2Status;
extern int fanSpeed;  // 0-255 for PWM control

extern unsigned long lastMotionTime;
extern unsigned long lastDataSend;
extern unsigned long lastLCDUpdate;
extern unsigned long lastAnimationUpdate;
extern unsigned long lastPingTime;
extern unsigned long lastDeviceUpdate;
extern uint8_t animationFrame;
extern uint8_t currentDisplayPage;  // For cycling through display pages

const unsigned long MOTION_TIMEOUT = 5000;      // 5 seconds for motion LED
const unsigned long DATA_SEND_INTERVAL = 5000;  // 5 seconds between API updates
const unsigned long LCD_UPDATE_INTERVAL = 1000; // 1 second between LCD updates
const unsigned long ANIMATION_INTERVAL = 250;   // 250ms between animation frames
const unsigned long PING_INTERVAL = 30000;      // 30 seconds between ping messages
const unsigned long DEVICE_UPDATE_INTERVAL = 1000; // 1 second between device status updates
const unsigned long DISPLAY_PAGE_INTERVAL = 5000; // 5 seconds between display pages

// LCD display states
enum LcdState {
    STARTING,
    CONNECTING_WIFI,
    AP_MODE,
    NORMAL_OPERATION,
    API_ERROR,
    SENSOR_ERROR
};

extern LcdState currentLcdState;

// IP Addresses for captive portal
extern const IPAddress localIP;
extern const IPAddress gatewayIP;
extern const IPAddress subnetMask;

#endif // CONFIG_H