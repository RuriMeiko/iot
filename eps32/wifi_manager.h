/*
 * File: wifi_manager.h
 * Mu00f4 tu1ea3: Quu1ea3n lu00fd ku1ebft nu1ed1i WiFi vu00e0 chu1ebf u0111u1ed9 Access Point
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ======= KHAI Bu00c1O Cu00c1C Hu00c0M =======

/**
 * Khu1edfi tu1ea1o module WiFi
 */
void setupWiFi();

/**
 * Ku1ebft nu1ed1i vu1edbi mu1ea1ng WiFi
 * @return true nu1ebfu ku1ebft nu1ed1i thu00e0nh cu00f4ng, false nu1ebfu thu1ea5t bu1ea1i
 */
bool connectToWiFi();

/**
 * Khu1edfi tu1ea1o chu1ebf u0111u1ed9 Access Point
 */
void startAP();

/**
 * Xu1eed lu00fd chu1ebf u0111u1ed9 Access Point
 * Cu1ea7n gu1ecdi hu00e0m nu00e0y trong vu00f2ng lu1eb7p chu00ednh nu1ebfu u0111ang u1edf chu1ebf u0111u1ed9 AP
 */
void handleAPMode();

/**
 * Kiu1ec3m tra xem WiFi u0111u00e3 ku1ebft nu1ed1i chu01b0a
 * @return true nu1ebfu u0111u00e3 ku1ebft nu1ed1i, false nu1ebfu chu01b0a ku1ebft nu1ed1i
 */
bool isWiFiConnected();

/**
 * Lu1ea5y tu00ean mu1ea1ng WiFi hiu1ec7n tu1ea1i
 * @return Tu00ean mu1ea1ng WiFi (SSID) u0111ang ku1ebft nu1ed1i
 */
String getWiFiSSID();

/**
 * Lu1ea5y tru1ea1ng thu00e1i WiFi
 * @return true nu1ebfu u0111ang ku1ebft nu1ed1i, false nu1ebfu khu00f4ng
 */
bool getWiFiStatus();

/**
 * Thay u0111u1ed5i thu00f4ng tin u0111u0103ng nhu1eadp WiFi
 * @param newSSID     Tu00ean mu1ea1ng WiFi mu1edbi
 * @param newPassword Mu1eadt khu1ea9u mu1ea1ng WiFi mu1edbi
 * @return true nu1ebfu ku1ebft nu1ed1i thu00e0nh cu00f4ng vu1edbi thu00f4ng tin mu1edbi, false nu1ebfu thu1ea5t bu1ea1i
 */
bool changeWiFiCredentials(const String& newSSID, const String& newPassword);

#endif // WIFI_MANAGER_H