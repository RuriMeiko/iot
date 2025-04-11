#include "wifi_manager.h"
#include "config.h"
#include "display.h"
#include "html_content.h"
#include "websocket_handler.h"
#include <WiFi.h>
#include <esp_wifi.h>

// Define global objects
DNSServer dnsServer;
AsyncWebServer server(80);

// Define IP addresses for captive portal
const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);

void setupWiFiConnection() {
    currentLcdState = CONNECTING_WIFI;
    updateLCD();

    // For now, we'll just start the captive portal
    setupCaptivePortal();
}

void setupCaptivePortal() {
    currentLcdState = AP_MODE;
    updateLCD();

    // Start the soft access point
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    WiFi.softAP(apSSID, apPassword, 6, 0, 4); // Channel 6, hidden=false, max_connection=4

    // Android fix for AMPDU
    esp_wifi_stop();
    esp_wifi_deinit();
    wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
    my_config.ampdu_rx_enable = false;
    esp_wifi_init(&my_config);
    esp_wifi_start();

    // Start DNS server
    dnsServer.setTTL(3600);
    dnsServer.start(53, "*", localIP);

    // Setup webserver routes
    setUpWebServer();

    server.begin();

    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.write(1);  // AP icon
    LCD.print(" AP: ");
    LCD.print(apSSID);
    LCD.setCursor(0, 1);
    LCD.print("IP: 4.3.2.1");
}

bool tryConnectWifi(String ssid, String password) {
    currentLcdState = CONNECTING_WIFI;
    updateLCD();

    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print("Connecting to:");
    LCD.setCursor(0, 1);
    LCD.print(ssid);

    WiFi.begin(ssid.c_str(), password.c_str());

    // Show connection animation
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        displayLoadingAnimation();
        delay(250);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        isWiFiConnected = true;
        currentLcdState = NORMAL_OPERATION;

        // Connection success animation
        LCD.clear();
        LCD.setCursor(0, 0);
        LCD.write(0);  // WiFi icon
        LCD.print(" Connected!");
        LCD.setCursor(0, 1);
        LCD.print(WiFi.localIP());

        // LED success pattern (3 quick blinks)
        for(int i = 0; i < 3; i++) {
            digitalWrite(LED, HIGH);
            delay(100);
            digitalWrite(LED, LOW);
            delay(100);
        }

        delay(2000);  // Display IP for 2 seconds

        // WebSocket connection removed as it's currently disabled
        // setupWebSocket();

        updateLCD();
        return true;
    } else {
        // Connection failure animation
        LCD.clear();
        LCD.setCursor(0, 0);
        LCD.print("WiFi Connection");
        LCD.setCursor(0, 1);
        LCD.print("Failed!");

        // LED error pattern (one long blink)
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED, LOW);

        delay(2000);  // Show error for 2 seconds
        currentLcdState = AP_MODE;
        updateLCD();
        return false;
    }
}

void handleScanRequest(AsyncWebServerRequest *request) {
    String json = "{\"networks\":[";
    int n = WiFi.scanComplete();
    if (n == -2) {
        WiFi.scanNetworks(true);
    } else if (n) {
        for (int i = 0; i < n; ++i) {
            if (i) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
        }
        WiFi.scanDelete();
        if (WiFi.scanComplete() == -2) {
            WiFi.scanNetworks(true);
        }
    }
    json += "]}";
    request->send(200, "application/json", json);
}

void handleConnectRequest(AsyncWebServerRequest *request) {
    String ssid, password;
    if (request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    }

    bool success = tryConnectWifi(ssid, password);

    String response = "{\"success\":" + String(success ? "true" : "false") + "}";
    request->send(200, "application/json", response);
}

void setUpWebServer() {
    // Basic captive portal detection routes
    server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
        request->redirect("http://logout.net");
    });

    server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
        request->send(404);
    });

    server.on("/generate_204", [](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
    });

    server.on("/redirect", [](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
    });

    server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
    });

    server.on("/canonical.html", [](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
    });

    server.on("/success.txt", [](AsyncWebServerRequest *request) {
        request->send(200);
    });

    server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
    });

    // Handle WiFi scanning and connection
    server.on("/scan", HTTP_GET, handleScanRequest);
    server.on("/connect", HTTP_POST, handleConnectRequest);

    // Main page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", portal_html);
        response->addHeader("Cache-Control", "public,max-age=31536000");
        request->send(response);
    });

    // 404 handler - redirect to main page
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
    });
}