/*
 * File: wifi_manager.cpp
 * Description: Manage WiFi connection and Access Point mode
 */

#include "wifi_manager.h"
#include "lcd_display.h"

// Global variables for WebServer and WiFi information
WebServer server(80);
String ssid = "";
String password = "";
boolean isConnected = false;

// Name of the Access Point when ESP32 cannot connect to WiFi
const char* AP_NAME = "Binh Minh o di gioi";

void setupWiFi() {
  // Start by remembering previous configurations
  // This part could use EEPROM, Preferences or SPIFFS for storage
  // Simple approach for demo is to leave it empty, will create AP each time on boot
  
  // Try to connect with saved WiFi credentials (if any)
  if (connectToWiFi()) {
    Serial.println("WiFi connection successful!");
  } else {
    // If unable to connect, create Access Point
    startAP();
  }
}

bool connectToWiFi() {
  // If no WiFi credentials, return false
  if (ssid.length() == 0) {
    return false;
  }
  
  // Display connection message on LCD
  displayWiFiConnecting("Connecting...", ssid);
  
  // Disconnect from current connection if any
  WiFi.disconnect();
  delay(100);
  
  // Setup Station mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // Wait for connection up to 5 seconds
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
    delay(500);
    Serial.print(".");
    // Update dots on LCD screen to show waiting status
    static int dots = 0;
    lcd.setCursor(14, 1);
    lcd.print(String(dots % 3 + 1, '.'));
    dots++;
  }
  
  Serial.println("");
  
  // Check connection result
  if (WiFi.status() == WL_CONNECTED) {
    isConnected = true;
    Serial.println("WiFi connection successful!");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP().toString());
    
    // Display information on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(0); // WiFi icon
    lcd.print(" Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(2000);
    
    return true;
  } else {
    isConnected = false;
    Serial.println("WiFi connection failed!");
    return false;
  }
}

void startAP() {
  // Display message on LCD
  displayWiFiConnecting("Creating WiFi AP", "");
  
  // Setup AP+Station mode (allows network scanning)
  WiFi.mode(WIFI_AP_STA);
  
  // Create Access Point
  WiFi.softAP(AP_NAME);
  
  // Display AP information on LCD
  displayAPMode(AP_NAME);
  lcd.setCursor(4, 1);
  lcd.print(WiFi.softAPIP().toString());
  
  Serial.println("Access Point created!");
  Serial.print("SSID: ");
  Serial.println(AP_NAME);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP().toString());
  
  // Scan surrounding WiFi networks
  WiFi.scanNetworks(true);
  
  // Setup routes for web server
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/connect", HTTP_POST, handleConnect);
  
  // Start server
  server.begin();
  Serial.println("Web server started");
}

void handleAPMode() {
  // Handle web server requests
  server.handleClient();
}

bool isWiFiConnected() {
  return isConnected && (WiFi.status() == WL_CONNECTED);
}

String getWiFiSSID() {
  return ssid;
}

bool getWiFiStatus() {
  return isConnected;
}

bool changeWiFiCredentials(const String& newSSID, const String& newPassword) {
  // Update new login information
  ssid = newSSID;
  password = newPassword;
  
  // Display message on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New WiFi config");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  
  // Reconnect to WiFi
  return connectToWiFi();
}

// ======= WEB SERVER HANDLER FUNCTIONS =======

void handleRoot() {
  String html = "<!DOCTYPE html>\n"
              "<html>\n"
              "<head>\n"
              "  <title>ESP32 WiFi Setup</title>\n"
              "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
              "  <meta charset='UTF-8'>\n"
              "  <style>\n"
              "    body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }\n"
              "    .container { max-width: 400px; margin: 0 auto; }\n"
              "    h1 { color: #0066cc; }\n"
              "    .btn { background: #0066cc; color: white; border: none; padding: 10px 15px; border-radius: 4px; cursor: pointer; }\n"
              "    ul { list-style-type: none; padding: 0; }\n"
              "    li { padding: 10px; margin-bottom: 5px; background: #f1f1f1; border-radius: 4px; cursor: pointer; }\n"
              "    li:hover { background: #e1e1e1; }\n"
              "  </style>\n"
              "</head>\n"
              "<body>\n"
              "  <div class='container'>\n"
              "    <h1>ESP32 WiFi Setup</h1>\n"
              "    <p>Scan for WiFi networks:</p>\n"
              "    <button class='btn' onclick='window.location.href="/scan"'>Scan WiFi</button>\n"
              "  </div>\n"
              "</body>\n"
              "</html>\n";
  server.send(200, "text/html", html);
}

void handleScan() {
  int n = WiFi.scanComplete();
  
  if (n == -2) {
    // Scan not started yet, start scan
    WiFi.scanNetworks(true);
    server.send(200, "text/plain", "Scanning...");
    return;
  } else if (n == -1) {
    // Scan in progress
    server.send(200, "text/plain", "Scanning...");
    return;
  }
  
  String html = "<!DOCTYPE html>\n"
              "<html>\n"
              "<head>\n"
              "  <title>Mu1ea1ng WiFi</title>\n"
              "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
              "  <meta charset='UTF-8'>\n"
              "  <style>\n"
              "    body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }\n"
              "    .container { max-width: 400px; margin: 0 auto; }\n"
              "    h1 { color: #0066cc; }\n"
              "    .network { padding: 10px; margin-bottom: 5px; background: #f1f1f1; border-radius: 4px; cursor: pointer; }\n"
              "    .network:hover { background: #e1e1e1; }\n"
              "    .form { display: none; margin-top: 20px; }\n"
              "    .form input { width: 100%; padding: 8px; margin-bottom: 10px; box-sizing: border-box; }\n"
              "    .form button { background: #0066cc; color: white; border: none; padding: 10px 15px; border-radius: 4px; cursor: pointer; }\n"
              "  </style>\n"
              "  <script>\n"
              "    function showForm(ssid) {\n"
              "      document.getElementById('ssidInput').value = ssid;\n"
              "      document.getElementById('passwordForm').style.display = 'block';\n"
              "      document.getElementById('networkList').style.display = 'none';\n"
              "    }\n"
              "  </script>\n"
              "</head>\n"
              "<body>\n"
              "  <div class='container'>\n"
              "    <h1>Available WiFi Networks</h1>\n"
              "    <div id='networkList'>\n";
  
  if (n == 0) {
    html += "      <p>No networks found!</p>\n";
  } else {
    for (int i = 0; i < n; ++i) {
      html += "      <div class='network' onclick='showForm("\"" + WiFi.SSID(i) + "\"")'>" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + " dBm)</div>\n";
    }
  }
  
  html += "    </div>\n"
         "    <div id='passwordForm' class='form'>\n"
         "      <form action='/connect' method='post'>\n"
         "        <input type='hidden' id='ssidInput' name='ssid'>\n"
         "        <label for='password'>Password:</label>\n"
         "        <input type='password' id='password' name='password'>\n"
         "        <button type='submit'>Connect</button>\n"
         "      </form>\n"
         "    </div>\n"
         "  </div>\n"
         "</body>\n"
         "</html>\n";
  
  server.send(200, "text/html", html);
  
  // Xu00f3a ku1ebft quu1ea3 scan u0111u1ec3 giu1ea3i phu00f3ng bu1ed9 nhu1edb
  WiFi.scanDelete();
  // Sau u0111u00f3 bu1eaft u0111u1ea7u scan lu1ea1i u0111u1ec3 cu1eadp nhu1eadt ku1ebft quu1ea3
  WiFi.scanNetworks(true);
}

void handleConnect() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String newSSID = server.arg("ssid");
    String newPassword = server.arg("password");
    
    String html = "<!DOCTYPE html>\n"
                "<html>\n"
                "<head>\n"
                "  <title>Connecting...</title>\n"
                "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
                "  <meta charset='UTF-8'>\n"
                "  <style>\n"
                "    body { font-family: Arial, sans-serif; margin: 0; padding: 20px; text-align: center; }\n"
                "    .container { max-width: 400px; margin: 0 auto; }\n"
                "    h1 { color: #0066cc; }\n"
                "  </style>\n"
                "</head>\n"
                "<body>\n"
                "  <div class='container'>\n"
                "    <h1>WiFi Connection</h1>\n"
                "    <p>Connecting to " + newSSID + "</p>\n"
                "    <p>ESP32 will start operating if connection is successful.</p>\n"
                "  </div>\n"
                "</body>\n"
                "</html>\n";
    
    server.send(200, "text/html", html);
    
    // Thay u0111u1ed5i thu00f4ng tin WiFi vu00e0 thu1eed ku1ebft nu1ed1i
    if (changeWiFiCredentials(newSSID, newPassword)) {
      // Nu1ebfu ku1ebft nu1ed1i thu00e0nh cu00f4ng, du1eebng chu1ebf u0111u1ed9 AP
      WiFi.softAPdisconnect(true);
      server.stop();
    }
  } else {
    server.send(400, "text/plain", "Missing SSID or password information");
  }
}