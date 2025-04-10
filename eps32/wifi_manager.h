/*
 * File: wifi_manager.h
 * Description: Manage WiFi connection and Access Point mode
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "common.h"

// Declaration of Web Server handler function prototypes
void handleRoot();
void handleScan();
void handleConnect();

// ======= FUNCTION DECLARATIONS =======

/**
 * Initialize WiFi module
 */
void setupWiFi();

/**
 * Connect to WiFi network
 * @return true if connection is successful, false if failed
 */
bool connectToWiFi();

/**
 * Initialize Access Point mode
 */
void startAP();

/**
 * Handle Access Point mode
 * This function should be called in the main loop when in AP mode
 */
void handleAPMode();

/**
 * Check if WiFi is connected
 * @return true if connected, false if not connected
 */
bool isWiFiConnected();

/**
 * Get current WiFi network name
 * @return Connected WiFi network name (SSID)
 */
String getWiFiSSID();

/**
 * Get WiFi status
 * @return true if connected, false if not
 */
bool getWiFiStatus();

/**
 * Change WiFi login information
 * @param newSSID     New WiFi network name
 * @param newPassword New WiFi password
 * @return true if successfully connected with new credentials, false if failed
 */
bool changeWiFiCredentials(const String& newSSID, const String& newPassword);

#endif // WIFI_MANAGER_H