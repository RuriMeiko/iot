#include "config.h"
#include "custom_characters.h"

// ========== CONFIGURATION ==========
const char* apSSID = "Smart Environment";
const char* apPassword = "12345678";
const char* WS_SERVER = "abc.xyz";  // WebSocket server address
const int WS_PORT = 80;             // WebSocket server port
const char* WS_URL = "/ws";         // WebSocket endpoint

// ========== CUSTOM CHARACTERS ==========
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

// Fan icon
byte fanIcon[8] = {
    0b00000, 0b01110, 0b01010, 0b11111, 0b10101, 0b00100, 0b01110, 0b00000
};

// Light icon
byte lightIcon[8] = {
    0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b01110, 0b01110, 0b00000
};