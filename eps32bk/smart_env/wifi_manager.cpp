/*
 * WiFi and Captive Portal implementation for Smart Environment Monitoring System
 */

#include "wifi_manager.h"
#include "display.h"
#include <Preferences.h>
Preferences prefs;
void setupWiFiConnection() {
    Serial.println("Setting up WiFi connection...");
    updateLCD(CONNECTING_WIFI);

    // For now, we'll just start the captive portal
    setupCaptivePortal();
}

void setupCaptivePortal() {
    Serial.println("Starting Captive Portal...");
    updateLCD(AP_MODE);
    prefs.begin("wifi", true);
    String savedSSID = prefs.getString("ssid", "");
    String savedPass = prefs.getString("pass", "");
    prefs.end();

    if (!savedSSID.isEmpty() && !savedPass.isEmpty()) {
    Serial.printf("Found saved credentials: %s\n", savedSSID.c_str());
    if (tryConnectWifi(savedSSID, savedPass)) return;
    }

    // Start the soft access point
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    WiFi.softAP(AP_SSID, AP_PASSWORD, 6, 0, 4); // Channel 6, hidden=false, max_connection=4

    Serial.printf("AP started: %s\n", AP_SSID);
    Serial.printf("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());

    // Android fix for AMPDU
    Serial.println("Applying Android AMPDU fix...");
    esp_wifi_stop();
    esp_wifi_deinit();
    wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
    my_config.ampdu_rx_enable = false;
    esp_wifi_init(&my_config);
    esp_wifi_start();

    // Start DNS server
    dnsServer.setTTL(3600);
    dnsServer.start(53, "*", localIP);
    Serial.println("DNS server started");

    // Setup webserver routes
    setUpWebServer();

    server.begin();
    Serial.println("Web server started");

    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.write(1);  // AP icon
    LCD.print(" AP: ");
    LCD.print(AP_SSID);
    LCD.setCursor(0, 1);
    LCD.print("IP: 4.3.2.1");
}

void setUpWebServer() {
    Serial.println("Setting up web server routes...");

    // Basic captive portal detection routes
    server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
        Serial.println("Received /connecttest.txt request - redirecting");
        request->redirect("http://logout.net");
    });

    server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
        Serial.println("Received /wpad.dat request - returning 404");
        request->send(404);
    });

    server.on("/generate_204", [](AsyncWebServerRequest *request) {
        Serial.println("Received /generate_204 request - redirecting");
        request->redirect("http://4.3.2.1");
    });

    server.on("/redirect", [](AsyncWebServerRequest *request) {
        Serial.println("Received /redirect request - redirecting");
        request->redirect("http://4.3.2.1");
    });

    server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
        Serial.println("Received /hotspot-detect.html request - redirecting");
        request->redirect("http://4.3.2.1");
    });

    server.on("/canonical.html", [](AsyncWebServerRequest *request) {
        Serial.println("Received /canonical.html request - redirecting");
        request->redirect("http://4.3.2.1");
    });

    server.on("/success.txt", [](AsyncWebServerRequest *request) {
        Serial.println("Received /success.txt request - sending 200");
        request->send(200);
    });

    server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
        Serial.println("Received /ncsi.txt request - redirecting");
        request->redirect("http://4.3.2.1");
    });

    // Handle WiFi scanning and connection
    server.on("/scan", HTTP_GET, handleScanRequest);
    server.on("/connect", HTTP_POST, handleConnectRequest);

    // Main page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Serving main portal page");
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", portal_html);
        response->addHeader("Cache-Control", "public,max-age=31536000");
        request->send(response);
    });

    // 404 handler - redirect to main page
    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.printf("404 Not Found: %s - redirecting\n", request->url().c_str());
        request->redirect("http://4.3.2.1");
    });

    Serial.println("Web server routes set up complete");
}

void handleScanRequest(AsyncWebServerRequest *request) {
    Serial.println("Handling WiFi scan request");
    String json = "{\"networks\":[";
    int n = WiFi.scanComplete();
    if (n == -2) {
        WiFi.scanNetworks(true);
        Serial.println("Started async WiFi scan");
    } else if (n) {
        Serial.printf("Found %d networks\n", n);
        for (int i = 0; i < n; ++i) {
            if (i) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
            Serial.printf("  %d: %s (%d dBm)\n", i+1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }
        WiFi.scanDelete();
        if (WiFi.scanComplete() == -2) {
            WiFi.scanNetworks(true);
            Serial.println("Started new async WiFi scan");
        }
    } else {
        Serial.printf("WiFi scan returned %d networks\n", n);
    }
    json += "]}";
    request->send(200, "application/json", json);
    Serial.println("Scan results sent to client");
}

void handleConnectRequest(AsyncWebServerRequest *request) {
    String ssid, password;
    if (request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    }

    Serial.printf("Received connection request for SSID: %s\n", ssid.c_str());

    bool success = tryConnectWifi(ssid, password);

    String response = "{\"success\":" + String(success ? "true" : "false") + "}";
    request->send(200, "application/json", response);
    Serial.printf("Connection attempt %s\n", success ? "successful" : "failed");
}

bool tryConnectWifi(String ssid, String password) {
    Serial.printf("Attempting to connect to WiFi SSID: %s\n", ssid.c_str());
    updateLCD(CONNECTING_WIFI);

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
        Serial.printf("Connection attempt %d/20\n", attempts);
    }

    if (WiFi.status() == WL_CONNECTED) {
        isWiFiConnected = true;

        Serial.println("WiFi connected successfully!");
        Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());

        // Stop AP services BEFORE updating LCD and saving prefs
        Serial.println("Stopping AP, DNS and Web Server...");
        dnsServer.stop();
        server.end(); // Stop the web server
        // WiFi.mode(WIFI_MODE_STA); // Explicitly set STA mode if needed

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
        updateLCD(NORMAL_OPERATION); // Should change state *after* stopping servers

        Serial.println("Saving WiFi credentials...");
        prefs.begin("wifi", false); // Open Preferences in Read/Write mode
        prefs.putString("ssid", ssid);
        prefs.putString("pass", password);
        prefs.end(); // Close Preferences
        Serial.println("Credentials saved.");

        return true; // Success
    } else {
        Serial.println("WiFi connection failed");
        Serial.printf("WiFi status: %d\n", WiFi.status());

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
        updateLCD(AP_MODE);
        return false;
    }

}

bool checkWiFiStatus() {
            Serial.println(WiFi.status()                                                                                                                                                                                                                                                                                                                                                                                 );

    if (WiFi.status() != WL_CONNECTED && isWiFiConnected) {
        Serial.println("WiFi connection lost - attempting to reconnect");
        isWiFiConnected = false;
        updateLCD(CONNECTING_WIFI);

        // Try to reconnect before going back to AP mode
        WiFi.reconnect();
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            displayLoadingAnimation();
            delay(500);
            attempts++;
            Serial.printf("Reconnection attempt %d/10\n", attempts);
        }

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Reconnection failed - starting captive portal");
            setupCaptivePortal();  // Restart captive portal if reconnection fails
            return false;
        } else {
            Serial.println("WiFi reconnected successfully");
            isWiFiConnected = true;
            updateLCD(NORMAL_OPERATION);
            return true;
        }
    }
    return isWiFiConnected;
}
