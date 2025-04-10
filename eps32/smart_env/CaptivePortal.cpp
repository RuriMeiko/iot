#include "CaptivePortal.h"

CaptivePortal::CaptivePortal() : server(80) {
    localIP = IPAddress(4, 3, 2, 1);
    gatewayIP = IPAddress(4, 3, 2, 1);
    subnetMask = IPAddress(255, 255, 255, 0);
    timeoutMinutes = 0; // Default: no timeout
    startTime = 0;

    // Default HTML if custom isn't set
    customHTML = R"=====(
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
                <div class="network">
                    <div class="network-name">Scanning networks...</div>
                </div>
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
        document.getElementById('wifi-form').addEventListener('submit', function(e) {
            e.preventDefault();
            var ssid = document.getElementById('ssid').value;
            var password = document.getElementById('password').value;

            document.getElementById('status').className = 'status connecting';
            document.getElementById('status').textContent = 'Connecting to ' + ssid + '...';

            fetch('/connect', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: 'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    document.getElementById('status').className = 'status success';
                    document.getElementById('status').textContent = 'Connected successfully! Redirecting...';
                } else {
                    document.getElementById('status').className = 'status error';
                    document.getElementById('status').textContent = 'Connection failed. Please try again.';
                }
            })
            .catch(error => {
                document.getElementById('status').className = 'status error';
                document.getElementById('status').textContent = 'An error occurred. Please try again.';
            });
        });

        // Scan for WiFi networks
        fetch('/scan')
            .then(response => response.json())
            .then(data => {
                var wifiList = document.getElementById('wifi-list');
                wifiList.innerHTML = '';

                data.networks.forEach(function(network) {
                    var div = document.createElement('div');
                    div.className = 'network';
                    div.innerHTML = '<div class="network-name">' + network.ssid + '</div>' +
                                   '<div class="signal">' + network.rssi + ' dBm</div>';
                    div.addEventListener('click', function() {
                        document.getElementById('ssid').value = network.ssid;
                    });
                    wifiList.appendChild(div);
                });
            });
    </script>
</body>
</html>
)=====";
}

void CaptivePortal::begin(const char* ssid, const char* password) {
    apSSID = String(ssid);
    if (password != NULL) {
        apPassword = String(password);
    } else {
        apPassword = "";
    }
}

void CaptivePortal::setAPName(const String& ssid) {
    apSSID = ssid;
}

void CaptivePortal::setAPPassword(const String& password) {
    apPassword = password;
}

void CaptivePortal::setCustomHTML(const String& html) {
    customHTML = html;
}

void CaptivePortal::setConnectionCallback(std::function<bool(String, String)> callback) {
    connectionCallback = callback;
}

void CaptivePortal::setTimeoutMins(int mins) {
    timeoutMinutes = mins;
}

void CaptivePortal::start() {
    // Start the soft access point
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);

    if (apPassword.length() > 0) {
        WiFi.softAP(apSSID.c_str(), apPassword.c_str(), 6, 0, 4); // Channel 6, hidden=false, max_connection=4
    } else {
        WiFi.softAP(apSSID.c_str(), NULL, 6, 0, 4);
    }
    
    // Record the start time for timeout functionality
    startTime = millis();

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
    setupWebserver();

    server.begin();
}

void CaptivePortal::setupWebserver() {
    // Required routes for captive portal detection
    server.on("/connecttest.txt", [this](AsyncWebServerRequest *request) {
        request->redirect("http://logout.net");
    });

    server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
        request->send(404);
    });

    // Routes for different platforms
    server.on("/generate_204", [this](AsyncWebServerRequest *request) {
        request->redirect("http://" + localIP.toString());
    });

    server.on("/redirect", [this](AsyncWebServerRequest *request) {
        request->redirect("http://" + localIP.toString());
    });

    server.on("/hotspot-detect.html", [this](AsyncWebServerRequest *request) {
        request->redirect("http://" + localIP.toString());
    });

    server.on("/canonical.html", [this](AsyncWebServerRequest *request) {
        request->redirect("http://" + localIP.toString());
    });

    server.on("/success.txt", [](AsyncWebServerRequest *request) {
        request->send(200);
    });

    server.on("/ncsi.txt", [this](AsyncWebServerRequest *request) {
        request->redirect("http://" + localIP.toString());
    });

    // Handle WiFi scanning
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
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
    });

    // Handle connection requests
    server.on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String ssid, password;
        if (request->hasParam("ssid", true)) {
            ssid = request->getParam("ssid", true)->value();
        }
        if (request->hasParam("password", true)) {
            password = request->getParam("password", true)->value();
        }

        bool success = false;
        if (connectionCallback) {
            success = connectionCallback(ssid, password);
        }

        String response = "{\"success\":" + String(success ? "true" : "false") + "}";
        request->send(200, "application/json", response);
    });

    // Main page
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", customHTML);
        response->addHeader("Cache-Control", "public,max-age=31536000");
        request->send(response);
    });

    // 404 handler - redirect to main page
    server.onNotFound([this](AsyncWebServerRequest *request) {
        request->redirect("http://" + localIP.toString());
    });
}

void CaptivePortal::loop() {
    dnsServer.processNextRequest();
    
    // Check for timeout if enabled
    if (timeoutMinutes > 0) {
        unsigned long currentTime = millis();
        // Convert minutes to milliseconds and check if timeout occurred
        if (currentTime - startTime > (timeoutMinutes * 60 * 1000)) {
            // If connection callback exists, trigger with empty credentials to signal timeout
            if (connectionCallback) {
                connectionCallback("", "");
            }
            // Reset timer
            startTime = currentTime;
        }
    }
}

bool CaptivePortal::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}
