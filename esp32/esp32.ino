#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WebSocketsServer.h>

// ===== PIN DEFINITIONS =====
#define LED1_PIN 13      // LED 1 - PWM controlled
#define LED2_PIN 12      // LED 2 - On/Off
#define LED3_PIN 14      // LED 3 - On/Off
#define LED4_PIN 27      // LED 4 - Motion indicator
#define DHT_PIN 26       // DHT11 temperature/humidity sensor
#define PIR_PIN 25       // Motion sensor
#define DHTTYPE DHT11    // DHT sensor type

// ===== CONSTANTS =====
#define LCD_ADDR 0x27    // I2C address for LCD
#define MOTION_TIMEOUT 5000      // Time before motion detection resets (ms)
#define DATA_SEND_INTERVAL 10000 // Time between API updates (ms)
#define LCD_UPDATE_INTERVAL 2000 // Time between LCD updates (ms)
#define ANIMATION_INTERVAL 250   // Time between loading animations (ms)

// ===== WIFI CONFIG =====
const char* apSSID = "Smart Home Hub";
const char* apPassword = "";
const char* API_ENDPOINT = "wss://websocket-server-ts-production.up.railway.app/";

// ===== GLOBAL VARIABLES =====
WebSocketsServer webSocket(81);
float temperature = 0;
float humidity = 0;
bool motionDetected = false;
int led1Intensity = 0;      // 0-255
bool led2State = false;
bool led3State = false;
bool isWiFiConnected = false;
bool isApiConnected = false;
unsigned long lastMotionTime = 0;
unsigned long lastDataSend = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastAnimationUpdate = 0;
uint8_t animationFrame = 0;

// ===== CUSTOM CHARACTERS =====
// WiFi connected icon
byte wifiIcon[8] = {
  0b00000, 0b00000, 0b00100, 0b01110, 0b10101, 0b00100, 0b00000, 0b00000
};
// AP mode icon
byte apIcon[8] = {
  0b00000, 0b01110, 0b10001, 0b10101, 0b10001, 0b01110, 0b00000, 0b00000
};
// Temperature icon
byte tempIcon[8] = {
  0b00100, 0b01010, 0b01010, 0b01010, 0b01110, 0b11111, 0b11111, 0b01110
};
// Humidity icon
byte humidityIcon[8] = {
  0b00100, 0b00100, 0b01010, 0b01010, 0b10001, 0b10001, 0b10001, 0b01110
};
// Motion icon
byte motionIcon[8] = {
  0b00000, 0b00100, 0b01110, 0b11111, 0b00100, 0b00100, 0b00000, 0b00000
};
// Loading animation frames
byte loadingIcons[3][8] = {
  {0b00000, 0b00000, 0b00000, 0b11100, 0b11100, 0b00000, 0b00000, 0b00000},
  {0b00000, 0b00000, 0b00000, 0b00111, 0b00111, 0b00000, 0b00000, 0b00000},
  {0b00000, 0b00000, 0b11100, 0b00000, 0b00000, 0b00111, 0b00000, 0b00000}
};

// LCD display states
enum LcdState {
  STARTING, CONNECTING_WIFI, AP_MODE, NORMAL_OPERATION, API_ERROR, SENSOR_ERROR
};
LcdState currentLcdState = STARTING;

// ===== GLOBAL OBJECTS =====
DHT dht(DHT_PIN, DHTTYPE);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
DNSServer dnsServer;
AsyncWebServer server(80);

// IP Addresses for captive portal
const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
// ===== LED CONTROL FUNCTIONS =====
/**
 * Controls the intensity of LED1 (0-255)
 */
void controlLed1Intensity(int intensity) {
  intensity = constrain(intensity, 0, 255);
  ledcWrite(0, intensity);
  led1Intensity = intensity;
  Serial.print("LED1 brightness: ");
  Serial.println(intensity);
}

/**
 * Controls LED2 state
 */
void controlLed2(bool state) {
  digitalWrite(LED2_PIN, state ? HIGH : LOW);
  led2State = state;
  Serial.print("LED2: ");
  Serial.println(state ? "ON" : "OFF");
}

/**
 * Controls LED3 state
 */
void controlLed3(bool state) {
  digitalWrite(LED3_PIN, state ? HIGH : LOW);
  led3State = state;
  Serial.print("LED3: ");
  Serial.println(state ? "ON" : "OFF");
}

void broadcastDeviceStatus() {
  static unsigned long lastBroadcast = 0;
  if (millis() - lastBroadcast < 2000) return;
  lastBroadcast = millis();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  bool motionDetected = digitalRead(PIR_PIN);

  Serial.println("Sending data to server:");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Motion: ");
  Serial.println(motionDetected ? "Yes" : "No");

  StaticJsonDocument<256> doc;
  doc["led1"] = led1Intensity;
  doc["led2"] = led2State;
  doc["led3"] = led3State;
  doc["motion"] = motionDetected;
  doc["temp"] = temperature;
  doc["hum"] = humidity;

  String json;
  serializeJson(doc, json);
  webSocket.broadcastTXT(json);
}
String scanWifiJson() {
  int n = WiFi.scanComplete();
  if (n == -2) {
    WiFi.scanNetworks(true);
    return "{\"scanning\":true}";
  } else if (n >= 0) {
    String json = "{\"networks\":[";
    for (int i = 0; i < n; ++i) {
      if (i) json += ",";
      json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]}";
    WiFi.scanDelete();
    if (WiFi.scanComplete() == -2) {
      WiFi.scanNetworks(true);
    }
    return json;
  } else {
    return "{\"networks\":[]}";
  }
}
// ===== HTML TEMPLATE =====
const char portal_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Kết nối WiFi</title>
  <style>
    :root {
      --primary: #4CAF50;
      --bg: #fdfdfd;
      --text: #1e1e1e;
      --card-bg: #fff;
      --border: #ddd;
      --error: #f44336;
      --radius: 12px;
    }

    @media (prefers-color-scheme: dark) {
      :root {
        --bg: #1e1e1e;
        --text: #f5f5f5;
        --card-bg: #2a2a2a;
        --border: #444;
      }
    }

    * {
      box-sizing: border-box;
    }

    body {
      margin: 0;
      font-family: "Segoe UI", sans-serif;
      background: var(--bg);
      color: var(--text);
      padding: 20px;
    }

    .container {
      max-width: 500px;
      margin: auto;
    }

    h1 {
      text-align: center;
      margin-bottom: 20px;
    }

    .card {
      background: var(--card-bg);
      padding: 20px;
      border-radius: var(--radius);
      box-shadow: 0 4px 8px rgba(0,0,0,0.08);
    }

    .btn {
      background: var(--primary);
      color: white;
      border: none;
      padding: 12px;
      width: 100%;
      font-size: 16px;
      border-radius: 8px;
      cursor: pointer;
      margin-top: 10px;
    }

    .btn:hover {
      background: #43a047;
    }

    #status {
      margin-bottom: 10px;
      font-weight: bold;
    }

    .network {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px;
      border-bottom: 1px solid var(--border);
      cursor: pointer;
    }

    .network:hover {
      background: rgba(0,0,0,0.05);
    }

    .wifi-strong::before,
    .wifi-good::before,
    .wifi-weak::before {
      content: '';
      display: inline-block;
      width: 20px;
      height: 20px;
      background-size: contain;
      background-repeat: no-repeat;
    }

    .wifi-strong::before {
      background-image: url('data:image/svg+xml,<svg fill="%234CAF50" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M2.1 9.6L3.5 11A13.94 13.94 0 0112 6c3.6 0 6.9 1.4 9.4 3.6l1.4-1.4C19.4 5.1 15.8 3.5 12 3.5S4.6 5.1 2.1 9.6z"/><path d="M5.1 12.6l1.4 1.4A9.91 9.91 0 0112 10c2.5 0 4.8 1 6.5 2.6l1.4-1.4A11.93 11.93 0 0012 8c-3 0-5.7 1.2-7.9 3.6z"/><path d="M8.1 15.6L12 19l3.9-3.4a5.98 5.98 0 00-7.8 0z"/></svg>');
    }

    .wifi-good::before {
      background-image: url('data:image/svg+xml,<svg fill="%23FF9800" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M5.1 12.6l1.4 1.4A9.91 9.91 0 0112 10c2.5 0 4.8 1 6.5 2.6l1.4-1.4A11.93 11.93 0 0012 8c-3 0-5.7 1.2-7.9 3.6z"/><path d="M8.1 15.6L12 19l3.9-3.4a5.98 5.98 0 00-7.8 0z"/></svg>');
    }

    .wifi-weak::before {
      background-image: url('data:image/svg+xml,<svg fill="%23F44336" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M8.1 15.6L12 19l3.9-3.4a5.98 5.98 0 00-7.8 0z"/></svg>');
    }

    #networks {
      max-height: 200px;
      overflow-y: auto;
      margin-top: 10px;
      border-top: 1px solid var(--border);
    }

    #connect-form {
      margin-top: 20px;
    }

    .pass-group {
      position: relative;
    }

    .pass-group input {
      width: 100%;
      padding: 10px 40px 10px 10px;
      border: 1px solid var(--border);
      border-radius: 8px;
      font-size: 16px;
    }

    .toggle-pass {
      position: absolute;
      right: 10px;
      top: 50%;
      transform: translateY(-50%);
      background: none;
      border: none;
      cursor: pointer;
      padding: 0;
    }

    .toggle-pass svg {
      width: 20px;
      height: 20px;
      fill: var(--text);
    }

    .status {
      color: var(--primary);
    }

    .error {
      color: var(--error);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Kết nối WiFi</h1>
    <div class="card">
      <div id="status">Đang quét mạng...</div>
      <button class="btn" onclick="scanNetworks()">🔍 Quét lại</button>
      <div id="networks"></div>

      <div id="connect-form" style="display:none;">
        <h3 id="selected-ssid"></h3>
        <div class="pass-group">
          <input type="password" id="password" placeholder="Nhập mật khẩu WiFi">
          <button class="toggle-pass" onclick="togglePassword(this)">
            <svg viewBox="0 0 24 24"><path d="M12 5c-7 0-11 7-11 7s4 7 11 7 11-7 11-7-4-7-11-7zm0 12c-2.8 0-5-2.2-5-5s2.2-5 5-5 5 2.2 5 5-2.2 5-5 5zm0-8a3 3 0 100 6 3 3 0 000-6z"/></svg>
          </button>
        </div>
        <button class="btn" onclick="connect()">🔗 Kết nối</button>
      </div>
    </div>
  </div>

  <script>
    let selectedSSID = '';

    function scanNetworks() {
      document.getElementById('status').innerText = 'Đang quét mạng...';
      fetch('/scan')
        .then(res => res.json())
        .then(data => {
          const networksEl = document.getElementById('networks');
          networksEl.innerHTML = '';
          data.networks.forEach(net => {
            let level = 'wifi-weak';
            if (net.rssi > -50) level = 'wifi-strong';
            else if (net.rssi > -70) level = 'wifi-good';
            networksEl.innerHTML += `
              <div class="network" onclick="selectNetwork('${net.ssid}')">
                <span>${net.ssid}</span>
                <span class="${level}"></span>
              </div>`;
          });
          document.getElementById('status').innerHTML = `<span class="status">Tìm thấy ${data.networks.length} mạng</span>`;
        })
        .catch(() => {
          document.getElementById('status').innerHTML = '<span class="error">Không thể quét mạng</span>';
        });
    }

    function selectNetwork(ssid) {
      selectedSSID = ssid;
      document.getElementById('selected-ssid').innerText = `SSID: ${ssid}`;
      document.getElementById('connect-form').style.display = 'block';
    }

    function connect() {
      const password = document.getElementById('password').value;
      const data = new FormData();
      data.append('ssid', selectedSSID);
      data.append('password', password);

      document.getElementById('status').innerHTML = `🔌 Đang kết nối tới ${selectedSSID}...`;

      fetch('/connect', {
        method: 'POST',
        body: data
      })
        .then(res => res.json())
        .then(data => {
          if (data.success) {
            document.getElementById('status').innerHTML = `<span class="status">✅ Đã kết nối tới ${selectedSSID}</span>`;
          } else {
            document.getElementById('status').innerHTML = `<span class="error">❌ Kết nối thất bại</span>`;
          }
        })
        .catch(() => {
          document.getElementById('status').innerHTML = `<span class="error">❌ Lỗi kết nối</span>`;
        });
    }

    function togglePassword(btn) {
      const input = btn.previousElementSibling;
      input.type = input.type === 'password' ? 'text' : 'password';
    }

    window.onload = scanNetworks;
  </script>
</body>
</html>

)=====";

// Forward declarations
void setupWiFi();
void setupCaptivePortal();
void setupWebServer();
void handleScanRequest(AsyncWebServerRequest *request);
void handleConnectRequest(AsyncWebServerRequest *request);
bool tryConnectWifi(String ssid, String password);
void readSensors();
void sendDataToServer();
void setupLCD();
void updateLCD();
void displayLoadingAnimation();
void controlLED(uint8_t pin, bool state);

// ===== SETUP FUNCTION =====
void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  Serial.println("\nSmart Home Hub Starting...");

  // GPIO Configuration
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  // Initialize PWM for LED1
  ledcAttachPin(LED1_PIN, 0); // Channel 0
  ledcSetup(0, 5000, 8);      // 5kHz, 8-bit
  ledcWrite(0, 0);            // Start with LED off

  // Initialize DHT sensor
  dht.begin();

  // Initialize SPIFFS for web files
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
  }

  // Initialize LCD
  setupLCD();

  // Initialize WiFi connection or Captive Portal
  setupWiFi();

  Serial.println("System initialization complete");
}

// ===== MAIN LOOP FUNCTION =====
void loop() {
  // For captive portal mode, handle DNS requests
  if (!isWiFiConnected) {
    dnsServer.processNextRequest();
  }

  // Read sensor data
  readSensors();

  // Update motion indicator
  if (motionDetected && millis() - lastMotionTime >= MOTION_TIMEOUT) {
    motionDetected = false;
    digitalWrite(LED4_PIN, LOW);
  }

  // Regular LCD updates
  if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
    updateLCD();
    lastLCDUpdate = millis();
  }

  // Loading animation for connecting screens
  if ((currentLcdState == CONNECTING_WIFI || currentLcdState == STARTING) &&
      millis() - lastAnimationUpdate >= ANIMATION_INTERVAL) {
    displayLoadingAnimation();
    lastAnimationUpdate = millis();
  }

  // Send data to server if connected
  if (isWiFiConnected && millis() - lastDataSend >= DATA_SEND_INTERVAL) {
    sendDataToServer();
    lastDataSend = millis();
  }

  // Reconnect if WiFi connection is lost
  if (isWiFiConnected && WiFi.status() != WL_CONNECTED) {
    isWiFiConnected = false;
    Serial.println("WiFi connection lost");

    // Try to reconnect before going back to AP mode
    WiFi.reconnect();
    delay(5000);

    if (WiFi.status() != WL_CONNECTED) {
      setupCaptivePortal();
    } else {
      isWiFiConnected = true;
    }
  }
  broadcastDeviceStatus();

  // Small delay to prevent CPU hogging
  delay(10);
}

// ===== LCD SETUP =====
void setupLCD() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, wifiIcon);
  lcd.createChar(1, apIcon);
  lcd.createChar(2, tempIcon);
  lcd.createChar(3, humidityIcon);
  lcd.createChar(4, motionIcon);
  lcd.createChar(5, loadingIcons[0]);
  lcd.createChar(6, loadingIcons[1]);
  lcd.createChar(7, loadingIcons[2]);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Smart Home Hub ");
  lcd.setCursor(0, 1);
  lcd.print(" Starting...    ");
  delay(1000);
}

// ===== WIFI SETUP =====
void setupWiFi() {
  currentLcdState = CONNECTING_WIFI;
  updateLCD();

  // Try to connect with saved credentials - replace with credentials loading from SPIFFS
  // WiFi.begin("saved_ssid", "saved_password");

  // Wait for connection for a maximum of 10 seconds
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    displayLoadingAnimation();
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    isWiFiConnected = true;
    currentLcdState = NORMAL_OPERATION;

    Serial.print("Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    // If connection failed, setup captive portal
    setupCaptivePortal();
  }
}

// ===== CAPTIVE PORTAL SETUP =====
void setupCaptivePortal() {
  currentLcdState = AP_MODE;
  updateLCD();

  WiFi.disconnect(true);
  delay(500);

  // Start access point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
  WiFi.softAP(apSSID, apPassword);

  // Start DNS server for captive portal
  dnsServer.setTTL(300);
  dnsServer.start(53, "*", localIP);

  // Setup web server
  setupWebServer();

  Serial.print("AP Started. SSID: ");
  Serial.println(apSSID);
  Serial.print("AP IP address: ");
  Serial.println(localIP);
}
void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(client_num);
      Serial.printf("Client [%u] connected from %s\n", client_num, ip.toString().c_str());

      // Gửi danh sách mạng ngay khi client kết nối
      String json = scanWifiJson();
      webSocket.sendTXT(client_num, json);
      break;
    }

    case WStype_TEXT: {
      String msg = String((char *)payload);
      Serial.printf("Received from client [%u]: %s\n", client_num, msg.c_str());

      if (msg == "scan") {
        String json = scanWifiJson();
        webSocket.sendTXT(client_num, json);

      } else if (msg.startsWith("connect:")) {
        int sep = msg.indexOf("|");
        String ssid = msg.substring(8, sep);
        String pass = msg.substring(sep + 1);
        bool success = tryConnectWifi(ssid, pass);
        webSocket.sendTXT(client_num, success ? "connected" : "failed");

      } else if (msg.startsWith("led1:")) {
        int val = msg.substring(5).toInt();
        controlLed1Intensity(val);

      } else if (msg.startsWith("led2:")) {
        bool state = msg.substring(5) == "1";
        controlLed2(state);

      } else if (msg.startsWith("led3:")) {
        bool state = msg.substring(5) == "1";
        controlLed3(state);
      }

      break;
    }

    case WStype_DISCONNECTED: {
      Serial.printf("Client [%u] disconnected\n", client_num);
      break;
    }

    default:
      break;
  }
}


// ===== WEB SERVER SETUP =====
void setupWebServer() {
  // Captive portal detection routes
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://4.3.2.1");
  });
  server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://4.3.2.1");
  });
  server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://4.3.2.1");
  });
  server.on("/fwlink", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://4.3.2.1");
  });

  // WiFi scanning and connection routes
  server.on("/scan", HTTP_GET, handleScanRequest);
  server.on("/connect", HTTP_POST, handleConnectRequest);

  // Main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", portal_html);
  });

//  // Sensor data API
//  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request) {
//    char jsonBuffer[128];
//    snprintf(jsonBuffer, sizeof(jsonBuffer),
//      "{\"temperature\":%.1f,\"humidity\":%.1f,\"motion\":%s,\"led1\":%d,\"led2\":%s,\"led3\":%s}",
//      temperature, humidity,
//      motionDetected ? "true" : "false",
//      led1Intensity,
//      led2State ? "true" : "false",
//      led3State ? "true" : "false");
//    request->send(200, "application/json", jsonBuffer);
//  });
//
//  // LED control API
//  server.on("/api/led1", HTTP_POST, [](AsyncWebServerRequest *request) {
//    if (request->hasParam("value", true)) {
//      int value = request->getParam("value", true)->value().toInt();
//      controlLed1Intensity(value);
//    }
//    request->send(200);
//  });
//
//  server.on("/api/led2", HTTP_POST, [](AsyncWebServerRequest *request) {
//    if (request->hasParam("state", true)) {
//      String state = request->getParam("state", true)->value();
//      controlLed2(state == "true" || state == "1");
//    }
//    request->send(200);
//  });
//
//  server.on("/api/led3", HTTP_POST, [](AsyncWebServerRequest *request) {
//    if (request->hasParam("state", true)) {
//      String state = request->getParam("state", true)->value();
//      controlLed3(state == "true" || state == "1");
//    }
//    request->send(200);
//  });

  // 404 handler - redirect to main page
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  server.begin();
}


// ===== WIFI SCAN HANDLER =====
void handleScanRequest(AsyncWebServerRequest *request) {
  int n = WiFi.scanComplete();
  if (n == -2) {
    WiFi.scanNetworks(true);
    request->send(200, "application/json", "{\"scanning\":true}");
  } else if (n >= 0) {
    String json = "{\"networks\":[";
    for (int i = 0; i < n; ++i) {
      if (i) json += ",";
      json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]}";
    WiFi.scanDelete();
    if (WiFi.scanComplete() == -2) {
      WiFi.scanNetworks(true);
    }
    request->send(200, "application/json", json);
  } else {
    request->send(200, "application/json", "{\"networks\":[]}");
  }
}

// ===== WIFI CONNECTION HANDLER =====
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

// ===== WIFI CONNECTION FUNCTION =====
bool tryConnectWifi(String ssid, String password) {
  if (ssid.length() == 0) return false;

  currentLcdState = CONNECTING_WIFI;
  updateLCD();

  // Disconnect from existing networks
  WiFi.disconnect(true);
  delay(500);

  // Try to connect
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.print("Connecting to ");
  Serial.print(ssid);

  // Wait for connection result (max 10 seconds)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    displayLoadingAnimation();
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    isWiFiConnected = true;
    currentLcdState = NORMAL_OPERATION;

    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());

    // Stop DNS server and AP mode
    dnsServer.stop();
    WiFi.mode(WIFI_STA);

    // Save credentials to SPIFFS (would be implemented here)
    return true;
  } else {
    Serial.println("Connection failed");
    currentLcdState = AP_MODE;
    updateLCD();
    return false;
  }
}

// ===== SENSOR READING FUNCTION =====
void readSensors() {
  // Read temperature and humidity
  float newTemp = dht.readTemperature();
  float newHum = dht.readHumidity();

  if (!isnan(newTemp) && !isnan(newHum)) {
    temperature = newTemp;
    humidity = newHum;
  }

  // Check for motion
  if (digitalRead(PIR_PIN) == HIGH) {
    motionDetected = true;
    lastMotionTime = millis();
    digitalWrite(LED4_PIN, HIGH);
  }
}

// ===== DATA SENDING FUNCTION =====
void sendDataToServer() {
  if (!isWiFiConnected) return;

  // This would be implemented with HTTPClient for actual API communication
  // For now, just simulate a successful API call
  Serial.println("Sending data to server:");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Motion: ");
  Serial.println(motionDetected ? "Yes" : "No");

  isApiConnected = true; // Simulate successful API connection
}

// ===== LCD UPDATE FUNCTION =====
void updateLCD() {
  switch (currentLcdState) {
    case STARTING:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Smart Home Hub ");
      lcd.setCursor(0, 1);
      lcd.print(" Starting...    ");
      break;

    case CONNECTING_WIFI:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Connecting WiFi ");
      lcd.setCursor(0, 1);
      lcd.print("Please wait.... ");
      break;

    case AP_MODE:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.write(1); // AP icon
      lcd.print(" Setup Mode    ");
      lcd.setCursor(0, 1);
      lcd.print(apSSID);
      break;

    case NORMAL_OPERATION:
      lcd.clear();
      // First row: WiFi status and temperature
      lcd.write(isWiFiConnected ? 0 : 1); // WiFi or AP icon
      lcd.setCursor(2, 0);
      lcd.write(2); // Temperature icon
      lcd.print(temperature, 1);
      lcd.print("C ");

      // Add motion indicator if detected
      if (motionDetected) {
        lcd.setCursor(15, 0);
        lcd.write(4); // Motion icon
      }

      // Second row: Humidity
      lcd.setCursor(2, 1);
      lcd.write(3); // Humidity icon
      lcd.print(humidity, 1);
      lcd.print("% ");

      // LED status indicators
      lcd.setCursor(10, 1);
      lcd.print("L:");
      lcd.print(led1Intensity > 0 ? "1" : "-");
      lcd.print(led2State ? "2" : "-");
      lcd.print(led3State ? "3" : "-");
      break;

    case API_ERROR:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("API Connection  ");
      lcd.setCursor(0, 1);
      lcd.print("Failed!         ");
      break;

    case SENSOR_ERROR:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor Error    ");
      lcd.setCursor(0, 1);
      lcd.print("Check DHT11     ");
      break;
  }
}

// ===== LOADING ANIMATION FUNCTION =====
void displayLoadingAnimation() {
  animationFrame = (animationFrame + 1) % 3;
  lcd.setCursor(15, 1);
  lcd.write(5 + animationFrame);
}
