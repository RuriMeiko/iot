/*
 * Smart Environment Monitoring System
 *
 * This system monitors temperature, humidity, and motion, displaying data on an LCD
 * and sending it to a cloud service. It uses a captive portal for easy WiFi setup.
 *
 * Components:
 * - ESP32 microcontroller
 * - DHT11 temperature/humidity sensor
 * - PIR motion sensor
 * - I2C LCD display (16x2)
 * - Status LED
 */

#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
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

// ========== CONFIGURATION ==========
const char* apSSID = "Smart Environment";
const char* apPassword = "12345678";
const char* API_ENDPOINT = "http://abc.xyz/data";

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

// ========== GLOBAL VARIABLES ==========
bool isWiFiConnected = false;
bool motionDetected = false;
bool isApiConnected = false;
unsigned long lastMotionTime = 0;
unsigned long lastDataSend = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastAnimationUpdate = 0;
uint8_t animationFrame = 0;

const unsigned long MOTION_TIMEOUT = 5000;      // 5 seconds for motion LED
const unsigned long DATA_SEND_INTERVAL = 5000;  // 5 seconds between API updates
const unsigned long LCD_UPDATE_INTERVAL = 1000; // 1 second between LCD updates
const unsigned long ANIMATION_INTERVAL = 250;   // 250ms between animation frames

// LCD display states
enum LcdState {
    STARTING,
    CONNECTING_WIFI,
    AP_MODE,
    NORMAL_OPERATION,
    API_ERROR,
    SENSOR_ERROR
};
LcdState currentLcdState = STARTING;

// ========== GLOBAL OBJECTS ==========
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C LCD(LCD_ADDR, 16, 2);
DNSServer dnsServer;
AsyncWebServer server(80);

// IP Addresses for captive portal
const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);

// Captive portal HTML template
const char portal_html[] PROGMEM = R"=====(
<!DOCTYPE html>
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
</html>
)=====";

// Function Prototypes
void displayWelcomeScreen();
void setupWiFiConnection();
void setupCaptivePortal();
void displayLoadingAnimation();
void sendDataToServer();
void updateLCD();
void displaySensorData();
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
    LCD.createChar(5, loadingIcon1);
    LCD.createChar(6, loadingIcon2);
    LCD.createChar(7, loadingIcon3);

    // Welcome splash screen
    displayWelcomeScreen();

    // Initialize sensors
    dht.begin();
    pinMode(PIRPIN, INPUT);
    pinMode(LED, OUTPUT);

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

        // LED success pattern (3
        // LED success pattern (3 quick blinks)
        for(int i = 0; i < 3; i++) {
            digitalWrite(LED, HIGH);
            delay(100);
            digitalWrite(LED, LOW);
            delay(100);
        }

        delay(2000);  // Display IP for 2 seconds
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

    switch(animationFrame) {
        case 0:
            LCD.setCursor(15, 1);
            LCD.write(5);  // Loading icon 1
            break;
        case 1:
            LCD.setCursor(15, 1);
            LCD.write(6);  // Loading icon 2
            break;
        case 2:
            LCD.setCursor(15, 1);
            LCD.write(7);  // Loading icon 3
            break;
    }
}

void sendDataToServer() {
    if (!isWiFiConnected) return;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        currentLcdState = SENSOR_ERROR;
        updateLCD();
        delay(2000);  // Show error briefly
        currentLcdState = NORMAL_OPERATION;
        return;
    }

    HTTPClient http;
    http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload using ArduinoJson 6.x syntax
    StaticJsonDocument<256> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["motion"] = motionDetected;
    doc["wifi_strength"] = WiFi.RSSI();
    doc["device_id"] = WiFi.macAddress();

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    int httpCode = http.POST(jsonPayload);

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
        isApiConnected = true;
        String response = http.getString();

        // Process possible server commands
        StaticJsonDocument<512> responseDoc;
        DeserializationError error = deserializeJson(responseDoc, response);

        if (!error && responseDoc.containsKey("new_wifi")) {
            String newSSID = responseDoc["new_wifi"]["ssid"];
            String newPassword = responseDoc["new_wifi"]["password"];

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
            } else {
                setupCaptivePortal(); // If new network fails, restart captive portal
            }
        }
    } else {
        isApiConnected = false;
        currentLcdState = API_ERROR;
        updateLCD();
        delay(2000);  // Show error briefly
        currentLcdState = NORMAL_OPERATION;
    }

    http.end();
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
            displaySensorData();
            break;

        case API_ERROR:
            LCD.clear();
            LCD.setCursor(0, 0);
            LCD.print("API Connection");
            LCD.setCursor(0, 1);
            LCD.print("Failed!");
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

    // API status indicator
    LCD.setCursor(14, 1);
    if (isApiConnected) {
        LCD.print("A");  // API connected
    } else {
        LCD.print("X");  // API disconnected
    }
}

void loop() {
    // Handle DNS server for Captive Portal
    dnsServer.processNextRequest();

    // Check motion sensor
    if (digitalRead(PIRPIN) == HIGH) {
        motionDetected = true;
        lastMotionTime = millis();
        digitalWrite(LED, HIGH);

        // Only update LCD immediately for motion events in normal operation
        if (currentLcdState == NORMAL_OPERATION) {
            updateLCD();
        }
    }

    // Turn off LED after timeout
    if (motionDetected && (millis() - lastMotionTime >= MOTION_TIMEOUT)) {
        motionDetected = false;
        digitalWrite(LED, LOW);

        // Update LCD when motion stops
        if (currentLcdState == NORMAL_OPERATION) {
            updateLCD();
        }
    }

    // Regular LCD updates
    if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
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
    if (isWiFiConnected && millis() - lastDataSend >= DATA_SEND_INTERVAL) {
        sendDataToServer();
        lastDataSend = millis();
    }

    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED && isWiFiConnected) {
        isWiFiConnected = false;
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
        }
    }

    // Small delay to prevent CPU hogging
    delay(10);
}
