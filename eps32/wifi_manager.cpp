/*
 * File: wifi_manager.cpp
 * Mu00f4 tu1ea3: Quu1ea3n lu00fd ku1ebft nu1ed1i WiFi vu00e0 chu1ebf u0111u1ed9 Access Point
 */

#include "wifi_manager.h"
#include "lcd_display.h"

// Biu1ebfn tou00e0n cu1ee5c cho WebServer vu00e0 thu00f4ng tin WiFi
WebServer server(80);
String ssid = "";
String password = "";
boolean isConnected = false;

// Tu00ean cu1ee7a Access Point khi ESP32 khu00f4ng ku1ebft nu1ed1i u0111u01b0u1ee3c WiFi
const char* AP_NAME = "Binh Minh o di gioi";

void setupWiFi() {
  // Bu1eaft u0111u1ea7u nhu1edb cu00e1c cu1ea5u hu00ecnh cu0169
  // Phu1ea7n nu00e0y cu00f3 thu1ec3 du00f9ng EEPROM, Preferences hou1eb7c SPIFFS u0111u1ec3 lu01b0u tru1eef
  // Cu00e1ch u0111u01a1n giu1ea3n cho demo lu00e0 u0111u1ec3 tru1ed1ng, su1ebd phu00e1t AP mu1ed7i lu1ea7n khu1edfi u0111u1ed9ng
  
  // Thu1eed ku1ebft nu1ed1i vu1edbi thu00f4ng tin WiFi u0111u00e3 lu01b0u (nu1ebfu cu00f3)
  if (connectToWiFi()) {
    Serial.println("Ku1ebft nu1ed1i WiFi thu00e0nh cu00f4ng!");
  } else {
    // Nu1ebfu khu00f4ng ku1ebft nu1ed1i u0111u01b0u1ee3c, tu1ea1o Access Point
    startAP();
  }
}

bool connectToWiFi() {
  // Nu1ebfu chu01b0a cu00f3 thu00f4ng tin WiFi, tru1ea3 vu1ec1 false
  if (ssid.length() == 0) {
    return false;
  }
  
  // Hiu1ec3n thu1ecb thu00f4ng bu00e1o ku1ebft nu1ed1i tru00ean LCD
  displayWiFiConnecting("Dang ket noi...", ssid);
  
  // Ngu1eaft ku1ebft nu1ed1i hiu1ec7n tu1ea1i nu1ebfu cu00f3
  WiFi.disconnect();
  delay(100);
  
  // Thiu1ebft lu1eadp chu1ebf u0111u1ed9 Station
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // u0110u1ee3i ku1ebft nu1ed1i tu1ed1i u0111a 5 giu00e2y
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
    delay(500);
    Serial.print(".");
    // Cu1eadp nhu1eadt du1ea5u chu1ea5m tru00ean mu00e0n hu00ecnh LCD u0111u1ec3 thu1ec3 hiu1ec7n tru1ea1ng thu00e1i u0111ang u0111u1ee3i
    static int dots = 0;
    lcd.setCursor(14, 1);
    lcd.print(String(dots % 3 + 1, '.'));
    dots++;
  }
  
  Serial.println("");
  
  // Kiu1ec3m tra ku1ebft quu1ea3 ku1ebft nu1ed1i
  if (WiFi.status() == WL_CONNECTED) {
    isConnected = true;
    Serial.println("Ku1ebft nu1ed1i WiFi thu00e0nh cu00f4ng!");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP().toString());
    
    // Hiu1ec3n thu1ecb thu00f4ng tin tru00ean LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(0); // WiFi icon
    lcd.print(" Da ket noi");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(2000);
    
    return true;
  } else {
    isConnected = false;
    Serial.println("Ku1ebft nu1ed1i WiFi thu1ea5t bu1ea1i!");
    return false;
  }
}

void startAP() {
  // Hiu1ec3n thu1ecb thu00f4ng bu00e1o tru00ean LCD
  displayWiFiConnecting("Tao diem phat WiFi", "");
  
  // Thiu1ebft lu1eadp chu1ebf u0111u1ed9 AP ku1ebft hu1ee3p Station (cho phu00e9p quu00e9t mu1ea1ng)
  WiFi.mode(WIFI_AP_STA);
  
  // Tu1ea1o Access Point
  WiFi.softAP(AP_NAME);
  
  // Hiu1ec3n thu1ecb thu00f4ng tin AP tru00ean LCD
  displayAPMode(AP_NAME);
  lcd.setCursor(4, 1);
  lcd.print(WiFi.softAPIP().toString());
  
  Serial.println("Access Point u0111u00e3 u0111u01b0u1ee3c tu1ea1o!");
  Serial.print("SSID: ");
  Serial.println(AP_NAME);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP().toString());
  
  // Quu00e9t cu00e1c mu1ea1ng WiFi xung quanh
  WiFi.scanNetworks(true);
  
  // Thiu1ebft lu1eadp cu00e1c route cho web server
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/connect", HTTP_POST, handleConnect);
  
  // Khu1edfi u0111u1ed9ng server
  server.begin();
  Serial.println("Web server u0111u00e3 u0111u01b0u1ee3c khu1edfi u0111u1ed9ng");
}

void handleAPMode() {
  // Xu1eed lu00fd cu00e1c yu00eau cu1ea7u cu1ee7a web server
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
  // Cu1eadp nhu1eadt thu00f4ng tin u0111u0103ng nhu1eadp mu1edbi
  ssid = newSSID;
  password = newPassword;
  
  // Hiu1ec3n thu1ecb thu00f4ng bu00e1o tru00ean LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cau hinh WiFi moi");
  lcd.setCursor(0, 1);
  lcd.print("Dang ket noi...");
  
  // Ku1ebft nu1ed1i lu1ea1i WiFi
  return connectToWiFi();
}

// ======= Cu00c1C Hu00c0M Xu1eec Lu00dd WEB SERVER =======

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
              "    <h1>Thiu1ebft lu1eadp WiFi ESP32</h1>\n"
              "    <p>Quu00e9t tu00ecm cu00e1c mu1ea1ng WiFi xung quanh:</p>\n"
              "    <button class='btn' onclick='window.location.href="/scan"'>Quu00e9t mu1ea1ng WiFi</button>\n"
              "  </div>\n"
              "</body>\n"
              "</html>\n";
  server.send(200, "text/html", html);
}

void handleScan() {
  int n = WiFi.scanComplete();
  
  if (n == -2) {
    // Scan chu01b0a u0111u01b0u1ee3c bu1eaft u0111u1ea7u, bu1eaft u0111u1ea7u scan
    WiFi.scanNetworks(true);
    server.send(200, "text/plain", "Scanning...");
    return;
  } else if (n == -1) {
    // Scan u0111ang tiu1ebfn hu00e0nh
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
              "    <h1>Mu1ea1ng WiFi khu1ea3 du1ee5ng</h1>\n"
              "    <div id='networkList'>\n";
  
  if (n == 0) {
    html += "      <p>Khu00f4ng tu00ecm thu1ea5y mu1ea1ng nu00e0o!</p>\n";
  } else {
    for (int i = 0; i < n; ++i) {
      html += "      <div class='network' onclick='showForm("\"" + WiFi.SSID(i) + "\"")'>" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + " dBm)</div>\n";
    }
  }
  
  html += "    </div>\n"
         "    <div id='passwordForm' class='form'>\n"
         "      <form action='/connect' method='post'>\n"
         "        <input type='hidden' id='ssidInput' name='ssid'>\n"
         "        <label for='password'>Mu1eadt khu1ea9u:</label>\n"
         "        <input type='password' id='password' name='password'>\n"
         "        <button type='submit'>Ku1ebft nu1ed1i</button>\n"
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
                "  <title>u0110ang ku1ebft nu1ed1i...</title>\n"
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
                "    <h1>Ku1ebft nu1ed1i WiFi</h1>\n"
                "    <p>u0110ang ku1ebft nu1ed1i tu1edbi " + newSSID + "</p>\n"
                "    <p>ESP32 su1ebd bu1eaft u0111u1ea7u hou1ea1t u0111u1ed9ng nu1ebfu ku1ebft nu1ed1i thu00e0nh cu00f4ng.</p>\n"
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
    server.send(400, "text/plain", "Thiu1ebfu thu00f4ng tin SSID hou1eb7c mu1eadt khu1ea9u");
  }
}