#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include "config.h"
#include "WateringZone.h"

// Web server instance
AsyncWebServer server(WEB_SERVER_PORT);

// Preferences for storing configuration
Preferences preferences;

// Watering zones
WateringZone* zones[NUM_ZONES];

// Time tracking
struct tm timeinfo;
uint8_t lastMinute = 255;  // Initialize to invalid value

// NTP Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for NTP time sync...");
  
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 10) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (getLocalTime(&timeinfo)) {
    Serial.println("\nTime synchronized");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  } else {
    Serial.println("\nFailed to obtain time");
  }
}

void setupZones() {
  uint8_t pins[NUM_ZONES] = {ZONE1_PIN, ZONE2_PIN, ZONE3_PIN, ZONE4_PIN};
  String defaultNames[NUM_ZONES] = {"Zone 1", "Zone 2", "Zone 3", "Zone 4"};
  
  for (int i = 0; i < NUM_ZONES; i++) {
    zones[i] = new WateringZone(pins[i], i + 1, defaultNames[i]);
    zones[i]->begin();
    
    // Load configuration from preferences
    preferences.begin(NVS_NAMESPACE, true);  // Read-only
    String nameKey = "zone" + String(i) + "_name";
    String savedName = preferences.getString(nameKey.c_str(), defaultNames[i]);
    zones[i]->setName(savedName);
    
    // Load schedules
    for (int j = 0; j < 3; j++) {
      String schedKey = "z" + String(i) + "_s" + String(j);
      bool enabled = preferences.getBool((schedKey + "_en").c_str(), false);
      uint8_t hour = preferences.getUChar((schedKey + "_h").c_str(), 0);
      uint8_t minute = preferences.getUChar((schedKey + "_m").c_str(), 0);
      uint32_t duration = preferences.getULong((schedKey + "_d").c_str(), DEFAULT_WATERING_DURATION);
      zones[i]->setSchedule(j, enabled, hour, minute, duration);
    }
    preferences.end();
  }
}

void saveZoneConfig(uint8_t zoneIndex) {
  if (zoneIndex >= NUM_ZONES) return;
  
  preferences.begin(NVS_NAMESPACE, false);  // Read-write
  String nameKey = "zone" + String(zoneIndex) + "_name";
  preferences.putString(nameKey.c_str(), zones[zoneIndex]->getName());
  
  // Save schedules
  for (int j = 0; j < 3; j++) {
    WateringSchedule* sched = zones[zoneIndex]->getSchedule(j);
    if (sched) {
      String schedKey = "z" + String(zoneIndex) + "_s" + String(j);
      preferences.putBool((schedKey + "_en").c_str(), sched->enabled);
      preferences.putUChar((schedKey + "_h").c_str(), sched->hour);
      preferences.putUChar((schedKey + "_m").c_str(), sched->minute);
      preferences.putULong((schedKey + "_d").c_str(), sched->duration);
    }
  }
  preferences.end();
}

void setupWebServer() {
  // Serve index page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Automated Watering System</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        h1 {
            color: white;
            text-align: center;
            margin-bottom: 30px;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .status-bar {
            background: white;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .status-bar h2 {
            color: #667eea;
            margin-bottom: 10px;
        }
        .zones-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .zone-card {
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            transition: transform 0.2s;
        }
        .zone-card:hover {
            transform: translateY(-5px);
        }
        .zone-card h3 {
            color: #667eea;
            margin-bottom: 15px;
        }
        .zone-status {
            display: flex;
            align-items: center;
            margin-bottom: 15px;
        }
        .status-indicator {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            margin-right: 10px;
        }
        .status-indicator.active {
            background: #10b981;
            box-shadow: 0 0 10px #10b981;
        }
        .status-indicator.inactive {
            background: #ef4444;
        }
        .controls {
            display: flex;
            gap: 10px;
            margin-top: 15px;
        }
        button {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 5px;
            font-size: 14px;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s;
        }
        .btn-start {
            background: #10b981;
            color: white;
        }
        .btn-start:hover {
            background: #059669;
        }
        .btn-stop {
            background: #ef4444;
            color: white;
        }
        .btn-stop:hover {
            background: #dc2626;
        }
        .btn-schedule {
            background: #667eea;
            color: white;
        }
        .btn-schedule:hover {
            background: #5568d3;
        }
        .duration-input {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            border: 2px solid #e5e7eb;
            border-radius: 5px;
            font-size: 14px;
        }
        .remaining-time {
            color: #6b7280;
            font-size: 14px;
            margin-top: 10px;
        }
        .schedule-section {
            background: #f9fafb;
            padding: 15px;
            border-radius: 5px;
            margin-top: 15px;
            display: none;
        }
        .schedule-section.active {
            display: block;
        }
        .schedule-item {
            background: white;
            padding: 10px;
            border-radius: 5px;
            margin-bottom: 10px;
        }
        .schedule-controls {
            display: grid;
            grid-template-columns: 60px 80px 80px 1fr;
            gap: 10px;
            align-items: center;
        }
        input[type="checkbox"] {
            width: 20px;
            height: 20px;
        }
        input[type="time"], input[type="number"] {
            padding: 8px;
            border: 1px solid #e5e7eb;
            border-radius: 4px;
        }
        .save-schedule {
            margin-top: 10px;
            width: 100%;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌱 Automated Watering System</h1>
        
        <div class="status-bar">
            <h2>System Status</h2>
            <p>WiFi: <span id="wifi-status">Connected</span></p>
            <p>Time: <span id="current-time">--:--</span></p>
            <p>IP Address: <span id="ip-address">--</span></p>
        </div>
        
        <div class="zones-grid" id="zones-container">
            <!-- Zones will be populated by JavaScript -->
        </div>
    </div>
    
    <script>
        let zones = [];
        
        async function fetchStatus() {
            try {
                const response = await fetch('/api/status');
                const data = await response.json();
                updateUI(data);
            } catch (error) {
                console.error('Error fetching status:', error);
            }
        }
        
        function updateUI(data) {
            zones = data.zones;
            document.getElementById('current-time').textContent = data.time || '--:--';
            document.getElementById('wifi-status').textContent = data.wifi ? 'Connected' : 'Disconnected';
            document.getElementById('ip-address').textContent = data.ip || '--';
            
            const container = document.getElementById('zones-container');
            container.innerHTML = '';
            
            data.zones.forEach(zone => {
                const card = createZoneCard(zone);
                container.appendChild(card);
            });
        }
        
        function createZoneCard(zone) {
            const card = document.createElement('div');
            card.className = 'zone-card';
            card.innerHTML = `
                <h3>${zone.name}</h3>
                <div class="zone-status">
                    <div class="status-indicator ${zone.active ? 'active' : 'inactive'}"></div>
                    <span>${zone.active ? 'Active' : 'Inactive'}</span>
                </div>
                ${zone.active ? `<div class="remaining-time">Remaining: ${zone.remaining}s</div>` : ''}
                <input type="number" class="duration-input" id="duration-${zone.id}" 
                       value="10" min="1" max="60" placeholder="Duration (minutes)">
                <div class="controls">
                    <button class="btn-start" onclick="startZone(${zone.id})">Start</button>
                    <button class="btn-stop" onclick="stopZone(${zone.id})">Stop</button>
                    <button class="btn-schedule" onclick="toggleSchedule(${zone.id})">Schedule</button>
                </div>
                <div class="schedule-section" id="schedule-${zone.id}">
                    <h4>Watering Schedules</h4>
                    ${createScheduleInputs(zone)}
                    <button class="btn-schedule save-schedule" onclick="saveSchedules(${zone.id})">Save Schedules</button>
                </div>
            `;
            return card;
        }
        
        function createScheduleInputs(zone) {
            let html = '';
            for (let i = 0; i < 3; i++) {
                const sched = zone.schedules[i];
                html += `
                    <div class="schedule-item">
                        <div class="schedule-controls">
                            <input type="checkbox" id="sched-${zone.id}-${i}-enabled" 
                                   ${sched.enabled ? 'checked' : ''}>
                            <input type="time" id="sched-${zone.id}-${i}-time" 
                                   value="${String(sched.hour).padStart(2,'0')}:${String(sched.minute).padStart(2,'0')}">
                            <input type="number" id="sched-${zone.id}-${i}-duration" 
                                   value="${sched.duration / 60000}" min="1" max="60" placeholder="Min">
                        </div>
                    </div>
                `;
            }
            return html;
        }
        
        function toggleSchedule(zoneId) {
            const section = document.getElementById(`schedule-${zoneId}`);
            section.classList.toggle('active');
        }
        
        async function startZone(zoneId) {
            const duration = document.getElementById(`duration-${zoneId}`).value;
            try {
                await fetch(`/api/zone/${zoneId}/start`, {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({duration: parseInt(duration) * 60000})
                });
                fetchStatus();
            } catch (error) {
                console.error('Error starting zone:', error);
            }
        }
        
        async function stopZone(zoneId) {
            try {
                await fetch(`/api/zone/${zoneId}/stop`, {method: 'POST'});
                fetchStatus();
            } catch (error) {
                console.error('Error stopping zone:', error);
            }
        }
        
        async function saveSchedules(zoneId) {
            const schedules = [];
            for (let i = 0; i < 3; i++) {
                const enabled = document.getElementById(`sched-${zoneId}-${i}-enabled`).checked;
                const time = document.getElementById(`sched-${zoneId}-${i}-time`).value;
                const duration = document.getElementById(`sched-${zoneId}-${i}-duration`).value;
                const [hour, minute] = time.split(':');
                
                schedules.push({
                    enabled: enabled,
                    hour: parseInt(hour),
                    minute: parseInt(minute),
                    duration: parseInt(duration) * 60000
                });
            }
            
            try {
                await fetch(`/api/zone/${zoneId}/schedule`, {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({schedules: schedules})
                });
                alert('Schedules saved successfully!');
                fetchStatus();
            } catch (error) {
                console.error('Error saving schedules:', error);
                alert('Error saving schedules');
            }
        }
        
        // Update status every 2 seconds
        setInterval(fetchStatus, 2000);
        fetchStatus();
    </script>
</body>
</html>
)rawliteral");
  });
  
  // API endpoint - Get system status
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(2048);
    
    // System info
    doc["wifi"] = WiFi.status() == WL_CONNECTED;
    doc["ip"] = WiFi.localIP().toString();
    
    // Current time
    if (getLocalTime(&timeinfo)) {
      char timeStr[20];
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
      doc["time"] = String(timeStr);
    }
    
    // Zone information
    JsonArray zonesArray = doc.createNestedArray("zones");
    for (int i = 0; i < NUM_ZONES; i++) {
      JsonObject zone = zonesArray.createNestedObject();
      zone["id"] = zones[i]->getId();
      zone["name"] = zones[i]->getName();
      zone["active"] = zones[i]->getStatus();
      zone["remaining"] = zones[i]->getRemainingTime();
      
      // Add schedules
      JsonArray schedArray = zone.createNestedArray("schedules");
      for (int j = 0; j < 3; j++) {
        WateringSchedule* sched = zones[i]->getSchedule(j);
        if (sched) {
          JsonObject schedObj = schedArray.createNestedObject();
          schedObj["enabled"] = sched->enabled;
          schedObj["hour"] = sched->hour;
          schedObj["minute"] = sched->minute;
          schedObj["duration"] = sched->duration;
        }
      }
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // API endpoint - Start zone
  server.on("/api/zone/*/start", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      String url = request->url();
      int zoneId = url.substring(10, url.indexOf("/start")).toInt();
      
      if (zoneId >= 1 && zoneId <= NUM_ZONES) {
        DynamicJsonDocument doc(256);
        deserializeJson(doc, (const char*)data);
        
        uint32_t duration = doc["duration"] | DEFAULT_WATERING_DURATION;
        zones[zoneId - 1]->start(duration);
        
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid zone\"}");
      }
    });
  
  // API endpoint - Stop zone
  server.on("/api/zone/*/stop", HTTP_POST, [](AsyncWebServerRequest *request){
    String url = request->url();
    int zoneId = url.substring(10, url.indexOf("/stop")).toInt();
    
    if (zoneId >= 1 && zoneId <= NUM_ZONES) {
      zones[zoneId - 1]->stop();
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Invalid zone\"}");
    }
  });
  
  // API endpoint - Set zone schedule
  server.on("/api/zone/*/schedule", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      String url = request->url();
      int zoneId = url.substring(10, url.indexOf("/schedule")).toInt();
      
      if (zoneId >= 1 && zoneId <= NUM_ZONES) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, (const char*)data);
        
        JsonArray schedules = doc["schedules"];
        for (int i = 0; i < schedules.size() && i < 3; i++) {
          JsonObject sched = schedules[i];
          zones[zoneId - 1]->setSchedule(
            i,
            sched["enabled"],
            sched["hour"],
            sched["minute"],
            sched["duration"]
          );
        }
        
        saveZoneConfig(zoneId - 1);
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid zone\"}");
      }
    });
  
  server.begin();
  Serial.println("Web server started");
}

void checkSchedules() {
  if (getLocalTime(&timeinfo)) {
    uint8_t currentMinute = timeinfo.tm_min;
    
    // Only check schedules once per minute
    if (currentMinute != lastMinute) {
      lastMinute = currentMinute;
      
      for (int i = 0; i < NUM_ZONES; i++) {
        zones[i]->checkSchedule(timeinfo.tm_hour, timeinfo.tm_min);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("Automated Watering System ESP32-S3");
  Serial.println("=================================\n");
  
  // Initialize zones
  setupZones();
  
  // Connect to WiFi
  setupWiFi();
  
  // Setup time synchronization
  if (WiFi.status() == WL_CONNECTED) {
    setupTime();
  }
  
  // Setup web server
  setupWebServer();
  
  Serial.println("\nSystem ready!");
  Serial.print("Access web interface at: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Update all zones
  for (int i = 0; i < NUM_ZONES; i++) {
    zones[i]->update();
  }
  
  // Check schedules
  checkSchedules();
  
  // Small delay to prevent watchdog issues
  delay(100);
}
