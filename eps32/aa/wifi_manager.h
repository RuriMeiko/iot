#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

// Function prototypes for WiFi management
void setupWiFiConnection();
void setupCaptivePortal();
bool tryConnectWifi(String ssid, String password);
void handleScanRequest(AsyncWebServerRequest *request);
void handleConnectRequest(AsyncWebServerRequest *request);
void setUpWebServer();

// External references for the server and DNS server
extern DNSServer dnsServer;
extern AsyncWebServer server;

#endif // WIFI_MANAGER_H