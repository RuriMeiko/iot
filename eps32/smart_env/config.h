/*
 * Configuration file for Smart Environment Monitoring System
 */

#ifndef CONFIG_H
#define CONFIG_H


// ========== PIN DEFINITIONS ==========
#define DHTPIN 15       // DHT temperature/humidity sensor
#define DHTTYPE DHT11
#define PIRPIN 18       // Motion sensor
#define LED 17          // Status LED
#define LCD_ADDR 0x27   // I2C address for LCD

// ========== CONFIGURATION ==========
#define AP_SSID "Smart Environment"
#define AP_PASSWORD ""
#define API_ENDPOINT "http://abc.xyz/data"

// ========== TIMING CONSTANTS ==========
#define MOTION_TIMEOUT 5000      // 5 seconds for motion LED
#define DATA_SEND_INTERVAL 5000  // 5 seconds between API updates
#define LCD_UPDATE_INTERVAL 1000 // 1 second between LCD updates
#define ANIMATION_INTERVAL 250   // 250ms between animation frames

// IP Addresses for captive portal
extern const IPAddress localIP;
extern const IPAddress gatewayIP;
extern const IPAddress subnetMask;


#endif // CONFIG_H
