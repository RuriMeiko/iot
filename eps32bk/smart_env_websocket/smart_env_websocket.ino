/*
 * Smart Environment Monitoring System with WebSocket
 *
 * This system monitors temperature, humidity, and motion, displaying data on an LCD
 * and communicating via WebSocket. It adds control for a fan and two LEDs.
 *
 * Components:
 * - ESP32 microcontroller
 * - DHT11 temperature/humidity sensor
 * - PIR motion sensor
 * - I2C LCD display (16x2)
 * - Status LED
 * - Fan (PWM control on pin 25)
 * - Two control LEDs (pins 26, 27)
 */

#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>
#include <WebSocketsClient.h> // WebSocket client library
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>  // Make sure to install ArduinoJson 6.x

// ========== PIN DEFINITIONS ==========
#define DHTPIN 14       // DHT temperature/humidity sensor
#define DHTTYPE DHT11
#define PIRPIN 18       // Motion sensor
#define LED 17          // Status LED
#define LCD_ADDR 0x27   // I2C address for LCD

// New device control pins
#define FAN_PIN 25      // Fan speed control (analog/PWM output)
#define LIGHT1_PIN 26   // Light 1 control (digital output)
#define LIGHT2_PIN 27   // Light 2 control (digital output)

// ========== CONFIGURATION ==========
const char* apSSID = "Smart Environment";
const char* apPassword = "";
const char* WS_SERVER = "wss://echo.websocket.events"; // Replace with your WebSocket server address
const int WS_PORT = 80;      // WebSocket server port
const char* WS_URL = "/ws";  // WebSocket URL path

// ========== CUSTOM CHARACTERS ==========
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
// Loading animation frames (3 frames)
byte loadingIcon1[8] = {
    0b00000, 0b00000, 0b00000, 0b11100, 0b11100, 0b00000, 0b00000, 0b00000
};
byte loadingIcon2[8] = {
    0b00000, 0b00000, 0b00000, 0b00111, 0b00111, 0b00000, 0b00000, 0b00000
};
byte loadingIcon3[8] = {
    0b00000, 0b00000, 0b11100, 0b00000, 0b00000, 0b00111, 0b00000, 0b00000
};
// Fan icon
byte fanIcon[8] = {
    0b00000, 0b01110, 0b01010, 0b11111, 0b10101, 0b00100, 0b01110, 0b00000
};
// Light icon
byte lightIcon[8] = {
    0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b01110, 0b01110, 0b00000
};

// ========== GLOBAL VARIABLES ==========
bool isWiFiConnected = false;
bool motionDetected = false;
bool isWsConnected = false;
bool light1Status = false;
bool light2Status = false;
int fanSpeed = 0;  // 0-255 for PWM control

unsigned long lastMotionTime = 0;
unsigned long lastDataSend = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastAnimationUpdate = 0;
unsigned long lastPingTime = 0;
unsigned long lastDeviceUpdate = 0;
uint8_t animationFrame = 0;
uint8_t currentDisplayPage = 0;  // For cycling through display pages

const unsigned long MOTION_TIMEOUT = 5000;      // 5 seconds for motion LED
const unsigned long DATA_SEND_INTERVAL = 5000;  // 5 seconds between API updates
const unsigned long LCD_UPDATE_INTERVAL = 1000; // 1 second between LCD updates
const unsigned long ANIMATION_INTERVAL = 250;   // 250ms between animation frames
const unsigned long PING_INTERVAL = 30000;      // 30 seconds between ping messages
const unsigned long DEVICE_UPDATE_INTERVAL = 1000; // 1 second between device status updates
const unsigned long DISPLAY_PAGE_INTERVAL = 5000; // 5 seconds between display pages

// LCD display states
enum LcdState {
    STARTING,
    CONNECTING_WIFI,
    AP_MODE,
    NORMAL_OPERATION,
    WS_ERROR,
    SENSOR_ERROR
};
LcdState currentLcdState = STARTING;

// ========== GLOBAL OBJECTS ==========
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C LCD(LCD_ADDR, 16, 2);
DNSServer dnsServer;
AsyncWebServer server(80);
WebSocketsClient webSocket;

// IP Addresses for captive portal
const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);

// Captive portal HTML template
const char portal_html[] PROGMEM =
R"=====(<!DOCTYPE html>
<html>
<head>
    <title>Smart Environment Hub</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body{font-family:'Helvetica Neue',Arial,sans-serif;margin:0;padding:0;color:#333;background:#f5f7fa}
        .container{max-width:460px;margin:20px auto;background:white;border-radius:12px;box-shadow:0 4px 15px rgba(0,0,0,0.1);overflow:hidden}
        .header{background:#4285f4;color:white;padding:20px;text-align:center}
        .content{padding:20px}
        h1{margin:0;font-weight:400;font-size:24px}
        h2{margin:0 0 20px;font-size:18px;font-weight:400;color:#666}
        .network-list{margin-bottom:15px;max-height:200px;overflow-y:auto}
        .network{padding:12px 15px;margin:8px 0;background:#f1f3f6;border-radius:6px;display:flex;align-items:center;cursor:pointer;transition:all 0.2s}
        .network:hover{background:#e1e5eb}
        .network-name{flex-grow:1;font-weight:500}
        .signal{font-size:14px;color:#888}
        .form-group{margin-bottom:15px}
        input{width:100%;padding:12px;border:1px solid #ddd;border-radius:4px;box-sizing:border-box;font-size:14px}
        button{background:#4285f4;color:white;border:none;padding:12px 20px;border-radius:4px;cursor:pointer;width:100%;font-size:16px;transition:background 0.2s}
        button:hover{background:#3b78e7}
        .status{margin-top:15px;padding:10px;border-radius:4px;text-align:center}
        .success{background:#d4edda;color:#155724}
        .error{background:#f8d7da;color:#721c24}
        @keyframes pulse{0%{opacity:1}50%{opacity:0.5}100%{opacity:1}}
        .connecting{animation:pulse 1.5s infinite;background:#f0f4c3;color:#5f6a00}
        .footer{text-align:center;padding:15px;color:#666;font-size:12px;border-top:1px solid #eee}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Smart Environment Hub</h1>
        </div>
        <div class="content">
            <h2>Connect to your home WiFi</h2>
            <div id="wifi-list" class="network-list">
                <!-- WiFi networks will be listed here -->
            </div>
            <form id="wifi-form">
                <div class="form-group">
                    <input type="text" id="ssid" name="ssid" placeholder="WiFi Name (SSID)" required>
                </div>
                <div class="form-group">
                    <input type="password" id="password" name="password" placeholder="WiFi Password">
                </div>
                <button type="submit">Connect</button>
            </form>
            <div id="status" class="status"></div>
        </div>
        <div class="footer">
            Smart Environment Hub &copy; 2023
        </div>
    </div>
    <script>
        // Load available networks
        fetch('/scan')
            .then(response => response.json())
            .then(data => {
                const wifiList = document.getElementById('wifi-list');
                wifiList.innerHTML = '';

                if (data.networks && data.networks.length > 0) {
                    data.networks.forEach(network => {
                        const div = document.createElement('div');
                        div.className = 'network';
                        div.innerHTML = `
                            <div class="network-name">${network.ssid}</div>
                            <div class="signal">${network.rssi} dBm</div>
                        `;
                        div.addEventListener('click', () => {
                            document.getElementById('ssid').value = network.ssid;
                        });
                        wifiList.appendChild(div);
                    });
                } else {
                    wifiList.innerHTML = '<div class="network"><div class="network-name">No networks found</div></div>';
                }
            })
            .catch(error => {
                console.error('Error scanning networks:', error);
            });

        // Handle form submission
        document.getElementById('wifi-form').addEventListener('submit', function(e) {
            e.preventDefault();
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;
            const status = document.getElementById('status');

            status.className = 'status connecting';
            status.textContent = `Connecting to ${ssid}...`;

            fetch('/connect', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}`
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    status.className = 'status success';
                    status.textContent = 'Connected successfully! The device will now restart.';
                } else {
                    status.className = 'status error';
                    status.textContent = 'Connection failed. Please try again.';
                }
            })
            .catch(error => {
                status.className = 'status error';
                status.textContent = 'An error occurred. Please try again.';
                console.error('Error connecting:', error);
            });
        });
    </script>
</body>
</html>)=====";

// Function Prototypes
void displayWelcomeScreen();
void setupWiFiConnection();
void setupCaptivePortal();
void setupWebSocket();
void setupDeviceControl();
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void displayLoadingAnimation();
void sendDataToServer();
void updateLCD();
void displaySensorData();
void displayDeviceStatus();
void handleDeviceControl(JsonDocument& doc);
void updateDeviceStatus();
bool tryConnectWifi(String ssid, String password);
void handleScanRequest(AsyncWebServerRequest *request);
void handleConnectRequest(AsyncWebServerRequest *request);
void setUpWebServer();

void setup() {
    Serial.begin(115200);
    Serial.println("\nSmart Environment Hub - Starting up...");

    // Initialize LCD
    LCD.init();
    LCD.backlight();
    LCD.createChar(0, wifiIcon);
    LCD.createChar(1, apIcon);
    LCD.createChar(2, tempIcon);
    LCD.createChar(3, humidityIcon);
    LCD.createChar(4, motionIcon);
    LCD.createChar(5, fanIcon);      // Replace loading icon 1 with fan icon
    LCD.createChar(6, lightIcon);    // Replace loading icon 2 with light icon
    LCD.createChar(7, loadingIcon3); // Keep loading icon 3 for animation

    // Welcome splash screen
    displayWelcomeScreen();

    // Initialize sensors and outputs
    dht.begin();
    pinMode(PIRPIN, INPUT);
    pinMode(LED, OUTPUT);

    // Setup device control pins
    setupDeviceControl();

    // Blink LED to indicate startup
    for(int i = 0; i < 3; i++) {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }

    // Try to connect to WiFi or start Captive Portal
    setupWiFiConnection();
}

void setupDeviceControl() {
    // Setup fan control
    pinMode(FAN_PIN, OUTPUT);
    analogWrite(FAN_PIN, 0);  // Start with fan off

    // Setup light controls
    pinMode(LIGHT1_PIN, OUTPUT);
    pinMode(LIGHT2_PIN, OUTPUT);
    digitalWrite(LIGHT1_PIN, LOW);
    digitalWrite(LIGHT2_PIN, LOW);
}

void displayWelcomeScreen() {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print(" Smart Home Hub ");
    LCD.setCursor(0, 1);
    LCD.print(" Initializing... ");
    delay(1500);
}

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

void setupWebSocket() {
    // Connect to WebSocket server
    webSocket.begin(WS_SERVER, WS_PORT, WS_URL);

    // Set WebSocket event handler
    webSocket.onEvent(webSocketEvent);

    // Set reconnect interval to 5 seconds
    webSocket.setReconnectInterval(5000);

    Serial.println("WebSocket connection established");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("WebSocket disconnected!");
            isWsConnected = false;
            break;

        case WStype_CONNECTED:
            Serial.println("WebSocket connected!");
            isWsConnected = true;

            // Send initial data after connection established
            sendDataToServer();
            break;

        case WStype_TEXT: {
            Serial.printf("WebSocket message received: %s\n", payload);

            // Process received message
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, (char*)payload);

            if (!error) {
                // Check for device control commands
                if (doc["device_control"].is<JsonObject>()) {
                    handleDeviceControl(doc);
                }
                // Check for WiFi update commands
                else if (doc["new_wifi"].is<JsonObject>()) {
                    String newSSID = doc["new_wifi"]["ssid"];
                    String newPassword = doc["new_wifi"]["password"];

                    // Display WiFi update notification
                    LCD.clear();
                    LCD.setCursor(0, 0);
                    LCD.print("New WiFi Config:");
                    LCD.setCursor(0, 1);
                    LCD.print(newSSID);
                    delay(2000);

                    // Connect to new network
                    WiFi.begin(newSSID.c_str(), newPassword.c_str());
                    int attempts = 0;

                    currentLcdState = CONNECTING_WIFI;
                    updateLCD();

                    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
                        displayLoadingAnimation();
                        delay(250);
                        attempts++;
                    }

                    if (WiFi.status() == WL_CONNECTED) {
                        isWiFiConnected = true;
                        currentLcdState = NORMAL_OPERATION;
                        setupWebSocket();  // Reconnect WebSocket with new WiFi
                    } else {
                        setupCaptivePortal(); // If new network fails, restart captive portal
                    }
                }
            }
            break;
        }

        case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            // Handle other WebSocket events if needed
            break;
    }
}

void handleDeviceControl(JsonDocument& doc) {
    // Parse fan control commands
    if (doc["device_control"]["fan"].is<int>()) {
        int speed = doc["device_control"]["fan"];
        // Ensure speed is within valid range (0-255)
        if (speed >= 0 && speed <= 255) {
            fanSpeed = speed;
            analogWrite(FAN_PIN, fanSpeed);
            Serial.printf("Fan speed set to %d\n", fanSpeed);
        }
    }

    // Parse light 1 control commands
    if (doc["device_control"]["light1"].is<bool>()) {
        bool state = doc["device_control"]["light1"];
        light1Status = state;
        digitalWrite(LIGHT1_PIN, light1Status ? HIGH : LOW);
        Serial.printf("Light 1 set to %s\n", light1Status ? "ON" : "OFF");
    }

    // Parse light 2 control commands
    if (doc["device_control"]["light2"].is<bool>()) {
        bool state = doc["device_control"]["light2"];
        light2Status = state;
        digitalWrite(LIGHT2_PIN, light2Status ? HIGH : LOW);
        Serial.printf("Light 2 set to %s\n", light2Status ? "ON" : "OFF");
    }

    // Update the device status display if we're in the right display mode
    if (currentLcdState == NORMAL_OPERATION && currentDisplayPage == 1) {
        displayDeviceStatus();
    }

    // Send status update back to server
    updateDeviceStatus();
}

void updateDeviceStatus() {
    if (!isWiFiConnected || !isWsConnected) return;

    JsonDocument doc;
    doc["device_status"]["fan"] = fanSpeed;
    doc["device_status"]["light1"] = light1Status;
    doc["device_status"]["light2"] = light2Status;
    doc["device_id"] = WiFi.macAddress();

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Send status update
    webSocket.sendTXT(jsonPayload);
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

        // Initialize WebSocket connection
        setupWebSocket();

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

void displayLoadingAnimation() {
    animationFrame = (animationFrame + 1) % 3;

    // We'll use the loading icon 3 for animation since 1 and 2 are now fan and light
    LCD.setCursor(15, 1);
    LCD.write(7);  // Loading icon
}

void sendDataToServer() {
    if (!isWiFiConnected || !isWsConnected) return;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        currentLcdState = SENSOR_ERROR;
        updateLCD();
        delay(2000);  // Show error briefly
        currentLcdState = NORMAL_OPERATION;
        return;
    }

    // Create JSON payload
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["motion"] = motionDetected;
    doc["wifi_strength"] = WiFi.RSSI();
    doc["device_id"] = WiFi.macAddress();
    doc["fan_speed"] = fanSpeed;
    doc["light1"] = light1Status;
    doc["light2"] = light2Status;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Send data through WebSocket
    webSocket.sendTXT(jsonPayload);
}

void updateLCD() {
    switch(currentLcdState) {
        case STARTING:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print(" Smart Home Hub ");
            LCD.setCursor(0, 1);
            LCD.print(" Initializing... ");
            break;

        case CONNECTING_WIFI:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("Connecting WiFi");
            LCD.setCursor(0, 1);
            LCD.print("Please wait");
            displayLoadingAnimation();
            break;

        case AP_MODE:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.write(1);  // AP icon
            LCD.print(" WiFi Setup Mode");
            LCD.setCursor(0, 1);
            LCD.print("Connect: ");
            LCD.print(apSSID);
            break;

        case NORMAL_OPERATION:
            // Rotate between sensor data and device status
            if (currentDisplayPage == 0) {
                displaySensorData();
            } else {
                displayDeviceStatus();
            }
            break;

        case WS_ERROR:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("WebSocket");
            LCD.setCursor(0, 1);
            LCD.print("Connection Error");
            break;

        case SENSOR_ERROR:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("Sensor Error");
            LCD.setCursor(0, 1);
            LCD.print("Check DHT11");
            break;
    }
}

void displaySensorData() {
    LCD.clear();

    // First row: WiFi status and temperature
    if (isWiFiConnected) {
        LCD.write(0);  // WiFi icon
    } else {
        LCD.write(1);  // AP icon
    }

    LCD.setCursor(2, 0);
    LCD.write(2);  // Temperature icon

    float temp = dht.readTemperature();
    if (!isnan(temp)) {
        LCD.print(temp, 1);
        LCD.print("C");
    } else {
        LCD.print("--.-C");
    }

    // Add motion indicator if detected
    if (motionDetected) {
        LCD.setCursor(14, 0);
        LCD.write(4);  // Motion icon
    }

    // Second row: Humidity and connection status
    LCD.setCursor(2, 1);
    LCD.write(3);  // Humidity icon

    float hum = dht.readHumidity();
    if (!isnan(hum)) {
        LCD.print(hum, 1);
        LCD.print("%");
    } else {
        LCD.print("--.-");
    }

    // WebSocket status indicator
    LCD.setCursor(14, 1);
    if (isWsConnected) {
        LCD.print("W");  // WebSocket connected
    } else {
        LCD.print("X");  // WebSocket disconnected
    }
}

void displayDeviceStatus() {
    LCD.clear();

    // First row: Fan status
    LCD.setCursor(0, 0);
    LCD.write(5);  // Fan icon
    LCD.print(" Fan: ");

    // Display fan speed as percentage
    int fanPercent = map(fanSpeed, 0, 255, 0, 100);
    LCD.print(fanPercent);
    LCD.print("%");

    // Second row: Light statuses
    LCD.setCursor(0, 1);
    LCD.write(6);  // Light icon
    LCD.print(" L1:");
    LCD.print(light1Status ? "ON" : "OFF");

    LCD.setCursor(9, 1);
    LCD.print("L2:");
    LCD.print(light2Status ? "ON" : "OFF");
}

void loop() {
    // Handle DNS server for Captive Portal
    dnsServer.processNextRequest();

    // Handle WebSocket if connected
    if (isWiFiConnected) {
        webSocket.loop();

        // Send ping to keep connection alive
        if (millis() - lastPingTime >= PING_INTERVAL) {
            if (isWsConnected) {
                webSocket.sendTXT("ping");
            }
            lastPingTime = millis();
        }
    }

    // Check motion sensor
    if (digitalRead(PIRPIN) == HIGH) {
        motionDetected = true;
        lastMotionTime = millis();
        digitalWrite(LED, HIGH);

        // Only update LCD immediately for motion events in normal operation
        if (currentLcdState == NORMAL_OPERATION && currentDisplayPage == 0) {
            updateLCD();
        }

        // Send motion event immediately through WebSocket if connected
        if (isWsConnected) {
            JsonDocument doc;
            doc["event"] = "motion_detected";
            doc["device_id"] = WiFi.macAddress();

            String jsonPayload;
            serializeJson(doc, jsonPayload);
            webSocket.sendTXT(jsonPayload);
        }
    }

    // Turn off LED after timeout
    if (motionDetected && (millis() - lastMotionTime >= MOTION_TIMEOUT)) {
        motionDetected = false;
        digitalWrite(LED, LOW);

        // Update LCD when motion stops
        if (currentLcdState == NORMAL_OPERATION && currentDisplayPage == 0) {
            updateLCD();
        }

        // Send motion stopped event through WebSocket if connected
        if (isWsConnected) {
            JsonDocument doc;
            doc["event"] = "motion_stopped";
            doc["device_id"] = WiFi.macAddress();

            String jsonPayload;
            serializeJson(doc, jsonPayload);
            webSocket.sendTXT(jsonPayload);
        }
    }

    // Regular LCD updates
    if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
        updateLCD();
        lastLCDUpdate = millis();
    }

    // Rotate display pages in normal operation
    if (currentLcdState == NORMAL_OPERATION && millis() - lastLCDUpdate >= DISPLAY_PAGE_INTERVAL) {
        currentDisplayPage = (currentDisplayPage + 1) % 2;  // Toggle between 0 and 1
        updateLCD();
        lastLCDUpdate = millis();
    }

    // Loading animation updates (faster than LCD)
    if (millis() - lastAnimationUpdate >= ANIMATION_INTERVAL &&
        (currentLcdState == CONNECTING_WIFI || currentLcdState == STARTING)) {
        displayLoadingAnimation();
        lastAnimationUpdate = millis();
    }

    // Send data to server
    if (isWiFiConnected && isWsConnected && millis() - lastDataSend >= DATA_SEND_INTERVAL) {
        sendDataToServer();
        lastDataSend = millis();
    }

    // Update device status
    if (isWiFiConnected && isWsConnected && millis() - lastDeviceUpdate >= DEVICE_UPDATE_INTERVAL) {
        updateDeviceStatus();
        lastDeviceUpdate = millis();
    }

    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED && isWiFiConnected) {
        isWiFiConnected = false;
        isWsConnected = false;
        currentLcdState = CONNECTING_WIFI;
        updateLCD();

        // Try to reconnect before going back to AP mode
        WiFi.reconnect();
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            displayLoadingAnimation();
            delay(500);
            attempts++;
        }

        if (WiFi.status() != WL_CONNECTED) {
            setupCaptivePortal();  // Restart captive portal if reconnection fails
        } else {
            isWiFiConnected = true;
            currentLcdState = NORMAL_OPERATION;
            setupWebSocket();  // Reconnect WebSocket
        }
    }

    // Small delay to prevent CPU hogging
    delay(10);
}
