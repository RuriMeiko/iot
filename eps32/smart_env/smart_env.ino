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

// WiFi.h is already included in CaptivePortal.h
#include <HTTPClient.h>
#include <ArduinoJson.h>
// Ensure we have JsonObject definition
#include <ArduinoJson/Variant/JsonObjectConst.hpp>
#include "CaptivePortal.h"
#include "LCDManager.h"
#include "SensorManager.h"

// ========== PIN DEFINITIONS ==========
#define DHTPIN 15       // DHT temperature/humidity sensor
#define DHTTYPE DHT11
#define PIRPIN 18       // Motion sensor
#define LED 17          // Status LED
#define LCD_ADDR 0x27   // I2C address for LCD

// ========== CONFIGURATION ==========
const char* apSSID = "Smart Environment";
const char* apPassword = "";
const char* API_ENDPOINT = "http://abc.xyz/data";

// ========== GLOBAL OBJECTS ==========
CaptivePortal captivePortal;
LCDManager lcdManager(LCD_ADDR, 16, 2);
SensorManager sensorManager(DHTPIN, DHTTYPE, PIRPIN, LED);

// ========== GLOBAL VARIABLES ==========
bool isWiFiConnected = false;
bool isApiConnected = false;
unsigned long lastDataSend = 0;
unsigned long lastLCDUpdate = 0;

const unsigned long MOTION_TIMEOUT = 5000;      // 5 seconds for motion LED
const unsigned long DATA_SEND_INTERVAL = 5000;  // 5 seconds between API updates
const unsigned long LCD_UPDATE_INTERVAL = 1000; // 1 second between LCD updates

// Current LCD display state
LcdState currentLcdState = STARTING;

void setup() {
    Serial.begin(115200);
    Serial.println("\nSmart Environment Hub - Starting up...");

    // Initialize LCD and show welcome screen
    lcdManager.init();
    lcdManager.displayWelcomeScreen();

    // Initialize sensors
    sensorManager.begin();

    // Setup WiFi connection
    setupWiFiConnection();
}

void setupWiFiConnection() {
    currentLcdState = CONNECTING_WIFI;
    lcdManager.updateDisplay(currentLcdState, 0, 0, false, false, false);

    // Start the captive portal for WiFi configuration
    setupCaptivePortal();
}

void setupCaptivePortal() {
    currentLcdState = AP_MODE;
    lcdManager.updateDisplay(currentLcdState, 0, 0, false, false, false);

    captivePortal.begin(apSSID, apPassword);
    captivePortal.setAPName(apSSID);
    captivePortal.setAPPassword(apPassword);
    captivePortal.setTimeoutMins(0);  // No timeout

    // Add HTML for a more professional looking portal
    captivePortal.setCustomHTML(R"=====(
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
        h2{margin:0 0 20px;font-size:18px;font-weight:
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
)=====");

    // Setup connection callback
    captivePortal.setConnectionCallback([](String ssid, String password) {
        currentLcdState = CONNECTING_WIFI;
        lcdManager.showWiFiConnecting(ssid);

        WiFi.begin(ssid.c_str(), password.c_str());

        // Show connection animation
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            lcdManager.displayLoadingAnimation();
            delay(250);
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            isWiFiConnected = true;
            currentLcdState = NORMAL_OPERATION;

            // Show connected status
            lcdManager.showWiFiConnected(WiFi.localIP().toString());

            // LED success pattern (3 quick blinks)
            for(int i = 0; i < 3; i++) {
                digitalWrite(LED, HIGH);
                delay(100);
                digitalWrite(LED, LOW);
                delay(100);
            }

            delay(2000); // Show IP for 2 seconds
            return true;
        } else {
            // Connection failure
            lcdManager.showWiFiError();

            // LED error pattern (one long blink)
            digitalWrite(LED, HIGH);
            delay(500);
            digitalWrite(LED, LOW);

            delay(2000); // Show error for 2 seconds
            currentLcdState = AP_MODE;
            return false;
        }
    });

    captivePortal.start();
    lcdManager.showAPMode(apSSID);
}

void sendDataToServer() {
    if (!isWiFiConnected) return;

    float temperature = sensorManager.readTemperature();
    float humidity = sensorManager.readHumidity();
    bool motion = sensorManager.getMotionState();

    if (isnan(temperature) || isnan(humidity)) {
        currentLcdState = SENSOR_ERROR;
        lcdManager.updateDisplay(currentLcdState, 0, 0, motion, isWiFiConnected, isApiConnected);
        delay(2000); // Show error briefly
        currentLcdState = NORMAL_OPERATION;
        return;
    }

    HTTPClient http;
    http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["motion"] = motion;
    doc["wifi_strength"] = WiFi.RSSI();
    doc["device_id"] = WiFi.macAddress();

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    int httpCode = http.POST(jsonPayload);

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
        isApiConnected = true;
        String response = http.getString();

        // Process possible server commands
        JsonDocument responseDoc;
        DeserializationError error = deserializeJson(responseDoc, response);

        if (!error && responseDoc["new_wifi"].is<JsonObject>()) {
            String newSSID = responseDoc["new_wifi"]["ssid"];
            String newPassword = responseDoc["new_wifi"]["password"];

            // Display WiFi update notification
            lcdManager.showWiFiConnecting(newSSID);

            // Connect to new network
            WiFi.begin(newSSID.c_str(), newPassword.c_str());
            int attempts = 0;

            currentLcdState = CONNECTING_WIFI;

            while (WiFi.status() != WL_CONNECTED && attempts < 20) {
                lcdManager.displayLoadingAnimation();
                delay(250);
                attempts++;
            }

            if (WiFi.status() == WL_CONNECTED) {
                isWiFiConnected = true;
                currentLcdState = NORMAL_OPERATION;
                lcdManager.showWiFiConnected(WiFi.localIP().toString());
                delay(2000);
            } else {
                setupCaptivePortal(); // If new network fails, restart captive portal
            }
        }
    } else {
        isApiConnected = false;
        currentLcdState = API_ERROR;
        lcdManager.updateDisplay(currentLcdState, 0, 0, motion, isWiFiConnected, isApiConnected);
        delay(2000); // Show error briefly
        currentLcdState = NORMAL_OPERATION;
    }

    http.end();
}

void loop() {
    // Handle Captive Portal if in AP mode
    captivePortal.loop();

    // Check and update motion sensor status
    sensorManager.detectMotion();
    sensorManager.updateMotionStatus(MOTION_TIMEOUT);

    // Update LCD at regular intervals
    if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
        float temp = sensorManager.readTemperature();
        float humid = sensorManager.readHumidity();
        bool motion = sensorManager.getMotionState();

        lcdManager.updateDisplay(currentLcdState, temp, humid, motion, isWiFiConnected, isApiConnected);
        lastLCDUpdate = millis();
    }

    // Send data to server at regular intervals
    if (isWiFiConnected && millis() - lastDataSend >= DATA_SEND_INTERVAL) {
        sendDataToServer();
        lastDataSend = millis();
    }

    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED && isWiFiConnected) {
        isWiFiConnected = false;
        currentLcdState = CONNECTING_WIFI;
        lcdManager.updateDisplay(currentLcdState, 0, 0, false, false, false);

        // Try to reconnect before going back to AP mode
        WiFi.reconnect();
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            lcdManager.displayLoadingAnimation();
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
