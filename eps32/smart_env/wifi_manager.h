/*
 * WiFi and Captive Portal management for Smart Environment Monitoring System
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "esp_wifi.h"
#include "config.h"

// Function declarations
void setupWiFiConnection();
void setupCaptivePortal();
void setUpWebServer();
void handleScanRequest(AsyncWebServerRequest *request);
void handleConnectRequest(AsyncWebServerRequest *request);
bool tryConnectWifi(String ssid, String password);
bool checkWiFiStatus();

extern DNSServer dnsServer;
extern AsyncWebServer server;
extern bool isWiFiConnected;
extern const char portal_html[];

#endif // WIFI_MANAGER_H
