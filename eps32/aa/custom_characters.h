#ifndef CUSTOM_CHARACTERS_H
#define CUSTOM_CHARACTERS_H

#include <Arduino.h>

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

#endif // CUSTOM_CHARACTERS_H