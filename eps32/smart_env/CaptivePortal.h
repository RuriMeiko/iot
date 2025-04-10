#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <functional>

class CaptivePortal {
  private:
    DNSServer dnsServer;
    AsyncWebServer server;
    IPAddress localIP;
    IPAddress gatewayIP;
    IPAddress subnetMask;
    String apSSID;
    String apPassword;
    String customHTML;
    std::function<bool(String, String)> connectionCallback;
    unsigned int timeoutMinutes;
    unsigned long startTime;
    
    void setupWebserver();

  public:
    CaptivePortal();
    void begin(const char* ssid, const char* password);
    void setAPName(const String& ssid);
    void setAPPassword(const String& password);
    void setCustomHTML(const String& html);
    void setConnectionCallback(std::function<bool(String, String)> callback);
    void setTimeoutMins(int mins);
    void loop();
    void start();
    bool isConnected();
};

#endif // CAPTIVE_PORTAL_H
