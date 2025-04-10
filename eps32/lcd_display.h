/*
 * File: lcd_display.h
 * Mu00f4 tu1ea3: Quu1ea3n lu00fd mu00e0n hu00ecnh LCD 16x2, bao gu1ed3m cu1ea5u hu00ecnh vu00e0 hiu1ec3n thu1ecb
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "common.h"

// u0110u1ecbnh nghu0129a cu00e1c custom character cho LCD
#define WIFI_ICON   0  // Biu1ec3u tu01b0u1ee3ng WiFi
#define TEMP_ICON   1  // Biu1ec3u tu01b0u1ee3ng nhiu1ec7t u0111u1ed9
#define HUMID_ICON  2  // Biu1ec3u tu01b0u1ee3ng u0111u1ed9 u1ea9m
#define MOTION_ICON 3  // Biu1ec3u tu01b0u1ee3ng chuyu1ec3n u0111u1ed9ng

// Khai bu00e1o biu1ebfn tou00e0n cu1ee5c
extern LiquidCrystal_I2C lcd;

// ======= KHAI Bu00c1O Cu00c1C Hu00c0M =======

/**
 * Khu1edfi tu1ea1o mu00e0n hu00ecnh LCD vu00e0 tu1ea1o custom characters
 */
void setupLCD();

/**
 * Hiu1ec3n thu1ecb mu00e0n hu00ecnh khu1edfi u0111u1ed9ng
 */
void displayBootScreen();

/**
 * Hiu1ec3n thu1ecb thu00f4ng tin vu1ec1 quu00e1 tru00ecnh ku1ebft nu1ed1i WiFi
 * @param message Thu00f4ng bu00e1o cu1ea7n hiu1ec3n thu1ecb
 * @param ssid    Tu00ean mu1ea1ng WiFi u0111ang ku1ebft nu1ed1i (tuu1ef3 chu1ecdn)
 */
void displayWiFiConnecting(const String& message, const String& ssid = "");

/**
 * Hiu1ec3n thu1ecb thu00f4ng tin vu1ec1 chu1ebf u0111u1ed9 AP
 * @param apName Tu00ean cu1ee7a Access Point
 */
void displayAPMode(const String& apName);

/**
 * Cu1eadp nhu1eadt thu00f4ng tin hiu1ec3n thu1ecb tru00ean LCD
 * @param temperature Nhiu1ec7t u0111u1ed9 (u00b0C)
 * @param humidity    u0110u1ed9 u1ea9m (%)
 * @param motion      Tru1ea1ng thu00e1i cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng
 * @param wifiStatus  Tru1ea1ng thu00e1i ku1ebft nu1ed1i WiFi
 */
void updateDisplay(float temperature, float humidity, bool motion, bool wifiStatus);

/**
 * Hiu1ec3n thu1ecb thu00f4ng bu00e1o lu1ed7i
 * @param error Thu00f4ng bu00e1o lu1ed7i cu1ea7n hiu1ec3n thu1ecb
 */
void displayError(const String& error);

#endif // LCD_DISPLAY_H