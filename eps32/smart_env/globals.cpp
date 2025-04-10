/*
 * Global definitions and variables for Smart Environment Monitoring System
 */

#include "globals.h"

// Define all global variables here
LiquidCrystal_I2C LCD(LCD_ADDR, 16, 2);

DNSServer dnsServer;
AsyncWebServer server(80);

bool isWiFiConnected = false;
bool motionDetected = false;
bool isApiConnected = false;

unsigned long lastMotionTime = 0;
unsigned long lastDataSend = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastAnimationUpdate = 0;

uint8_t animationFrame = 0;

LcdState currentLcdState = STARTING;

// IP Addresses for captive portal
const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);

// Captive portal HTML template (now a single declaration moved from wifi_manager.cpp)
const char portal_html[] PROGMEM = R"=====(<!DOCTYPE html>
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
