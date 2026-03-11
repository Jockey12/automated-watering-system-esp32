> Automated irrigation system for an organic farm.
> **Raspberry Pi 3** (web server + MQTT broker) + **4 × ESP32-S3** (WiFi connected).

---

## 1. System Architecture

```
┌──────────────────────────────┐
│  Raspberry Pi 3              │
│  Flask web server :5000      │
│  Mosquitto MQTT broker :1883 │
│  Connected to WiFi router    │
└──────────┬───────────────────┘
           │  WiFi / MQTT
    ┌──────┼──────┬──────┐
    ▼      ▼      ▼      ▼
┌──────┐┌──────┐┌──────┐┌──────┐
│ ESP1 ││ ESP2 ││ ESP3 ││ ESP4 │
│4relay││8relay││4relay││4relay│
│Pump  ││V1-V7 ││V8-11 ││V12-15│
│+DHT  ││+City ││      ││      │
│+Soil ││      ││      ││      │
└──────┘└──────┘└──────┘└──────┘
```

**Important:** All ESPs must connect to the same WiFi network. If WiFi range is insufficient, use a WiFi repeater/extender.

---

## 2. Pin Connections — All Connections

### 2a. ESP1 — Pump Controller + Sensors

3 of the 4 relay module channels are used (1 spare).

| Connection | ESP32-S3 GPIO | Function |
|------------|---------------|----------|
| Relay IN1 | **GPIO 4** | Breaker 1 (pump contactor 1) |
| Relay IN2 | **GPIO 5** | Breaker 2 (pump contactor 2) |
| Relay IN3 | **GPIO 6** | Breaker 3 (pump contactor 3) |
| Relay VCC | **5V** (from USB) | Relay coil power |
| Relay GND | **GND** | Common ground |
| DHT DATA | **GPIO 7** | Temperature/humidity sensor data pin |
| DHT VCC | **3.3V** | Sensor power |
| DHT GND | **GND** | Sensor ground |
| Soil Moisture | **GPIO 1** | Soil moisture sensor (analog input) |
| Soil VCC | **3.3V** | Sensor power |
| Soil GND | **GND** | Sensor ground |

**GPIOs used:** 1, 4, 5, 6, 7 = **5 total**

> **About the DHT sensor:** Code uses DHT11 (blue module) by default.
> If using DHT22 (white module), change `#define DHT_TIPI DHT11`
> to `DHT22` in `esp1_gateway.ino`.

---

### 2b. ESP2 — 8-Channel Valve Controller (Valves 1-7 + City Water)

| Connection | ESP32-S3 GPIO | Valve No | Controls |
|------------|---------------|----------|----------|
| Relay IN1 | **GPIO 4** | Valve 1 | Irrigation Zone 1 |
| Relay IN2 | **GPIO 5** | Valve 2 | Irrigation Zone 2 |
| Relay IN3 | **GPIO 6** | Valve 3 | Irrigation Zone 3 |
| Relay IN4 | **GPIO 7** | Valve 4 | Irrigation Zone 4 |
| Relay IN5 | **GPIO 15** | Valve 5 | Irrigation Zone 5 |
| Relay IN6 | **GPIO 16** | Valve 6 | Irrigation Zone 6 |
| Relay IN7 | **GPIO 17** | Valve 7 | Irrigation Zone 7 |
| Relay IN8 | **GPIO 18** | **City Water** | ⚠️ City water valve — **DO NOT OPEN SIMULTANEOUSLY WITH PUMP!** |
| Relay VCC | **5V** | | Relay coil power |
| Relay GND | **GND** | | Common ground |

**GPIOs used:** 4, 5, 6, 7, 15, 16, 17, 18 = **8 total**

---

### 2c. ESP3 — 4-Channel Valve Controller (Valves 8-11)

| Connection | ESP32-S3 GPIO | Valve No | Controls |
|------------|---------------|----------|----------|
| Relay IN1 | **GPIO 4** | Valve 8 | Irrigation Zone 8 |
| Relay IN2 | **GPIO 5** | Valve 9 | Irrigation Zone 9 |
| Relay IN3 | **GPIO 6** | Valve 10 | Irrigation Zone 10 |
| Relay IN4 | **GPIO 7** | Valve 11 | Irrigation Zone 11 |
| Relay VCC | **5V** | | Relay coil power |
| Relay GND | **GND** | | Common ground |

**GPIOs used:** 4, 5, 6, 7 = **4 total**

---

### 2d. ESP4 — 4-Channel Valve Controller (Valves 12-15)

| Connection | ESP32-S3 GPIO | Valve No | Controls |
|------------|---------------|----------|----------|
| Relay IN1 | **GPIO 4** | Valve 12 | Irrigation Zone 12 |
| Relay IN2 | **GPIO 5** | Valve 13 | Irrigation Zone 13 |
| Relay IN3 | **GPIO 6** | Valve 14 | Irrigation Zone 14 |
| Relay IN4 | **GPIO 7** | Valve 15 | Irrigation Zone 15 |
| Relay VCC | **5V** | | Relay coil power |
| Relay GND | **GND** | | Common ground |

**GPIOs used:** 4, 5, 6, 7 = **4 total**

---

### 2e. Power Connections

| Device | Power Supply | Notes |
|--------|-------------|-------|
| Raspberry Pi 3 | 5V / 2.5A micro-USB | Official power adapter recommended |
| Each ESP32-S3 | 5V USB (USB-C or micro) | |
| Relay modules VCC | 5V (from ESP's USB power rail) | |
| DHT sensor | 3.3V (ESP's 3V3 pin) | |
| Soil moisture sensor | 3.3V (ESP's 3V3 pin) | |

> **IMPORTANT:** The ESP's GND and the relay module's GND **must be connected together**
> (common ground). Relays will not work without this.

---

## 3. Safety Rules

### ⚠️ CRITICAL: Pump ↔ City Water Mutual Interlock

| Rule | Reason |
|------|--------|
| **Pump and city water MUST NOT be opened simultaneously** | Pipes burst (double pressure) |
| Server provides 3 layers of protection | `pump_start()`, `city_water_open()`, `/api/mode` |
| When source changes, the other is closed first | with 6-second pressure relief delay |

### Pump Start Sequence (protects the motor)
| Step | Wait | Description |
|------|------|-------------|
| Breaker 1 ON | 10 s | Soft start |
| Breaker 2 ON | 4 s | Gradual power increase |
| Breaker 3 ON | 4 s | Full power |
| **Total start time** | **~18 s** | |

### Pump Stop Sequence (reverse order)
| Step | Wait | Description |
|------|------|-------------|
| Motor stop command | 1 s | |
| Breaker 3 OFF | 3 s | Cooling |
| Breaker 2 OFF | 3 s | |
| Breaker 1 OFF | — | Done |
| **Total stop time** | **~10 s** | |

### Valve Timing
| Operation | Wait | Description |
|-----------|------|-------------|
| Open valve → start source | 6 s | Pipes filling with water |
| Close last valve → stop source | 6 s | Line pressure relief |

---

## 4. Relay Polarity Note

Code uses `HIGH = relay ON`, `LOW = relay OFF`. There are two types of relay modules:

| Type | Trigger | How to Identify |
|------|---------|-----------------|
| **Active-HIGH** (code default) | HIGH (3.3V) → relay ON | Relay clicks when pin goes HIGH |
| **Active-LOW** (some Chinese modules) | LOW (0V) → relay ON | Relays stay ON at startup |

**If your relays are active-LOW**, change these lines in each `.ino` file:

```cpp
// From:
digitalWrite(pin, state ? HIGH : LOW);   // setValve / setBreaker
digitalWrite(RELAY_PINS[i], LOW);        // closeAll / initRelays

// To:
digitalWrite(pin, state ? LOW : HIGH);   // reversed
digitalWrite(RELAY_PINS[i], HIGH);       // reversed
```

**Quick test:** Upload firmware and open Serial Monitor. If relays stay ON without any commands, the module is active-LOW.

---

## 5. MQTT Topics

| Topic | Direction | Example Message |
|-------|-----------|-----------------|
| `sulama/esp1/komut` | Pi → ESP1 | `{"cmd":"SIGORTA_AC","no":1}` |
| `sulama/esp2/komut` | Pi → ESP2 | `{"cmd":"VALF_AC","valf":3}` |
| `sulama/esp3/komut` | Pi → ESP3 | `{"cmd":"VALF_AC","valf":9}` |
| `sulama/esp4/komut` | Pi → ESP4 | `{"cmd":"VALF_AC","valf":14}` |
| `sulama/tum/komut` | Pi → all | `{"cmd":"TUMU_KAPAT"}` |
| `sulama/esp1/durum` | ESP1 → Pi | Breaker states |
| `sulama/esp2/durum` | ESP2 → Pi | Valve states |
| `sulama/esp3/durum` | ESP3 → Pi | Valve states |
| `sulama/esp4/durum` | ESP4 → Pi | Valve states |
| `sulama/esp{N}/heartbeat` | ESP → Pi | `{"id":"esp1_pompa","active":true,"wifi":-60}` (every 30s) |
| `sulama/sensor/data` | ESP1 → Pi | `{"temp":25,"hum":60,"soil":45}` |

---

## 6. Setup Steps

### 6a. Raspberry Pi Setup

**Requirements:** Raspberry Pi 3 with Raspberry Pi OS Lite (or Desktop), WiFi/Ethernet connected, SSH enabled.

```bash
# 1. SSH into the Pi
ssh pi@<PI_IP_ADDRESS>

# 2. Copy project to Pi (from your computer):
scp -r irrigation/ pi@<PI_IP_ADDRESS>:~/

# 3. If using Tailscale:
scp -r irrigation/ pi@<TAILSCALE_IP>:~/

# 4. Run the setup script on the Pi:
cd ~/irrigation
chmod +x setup_pi.sh
sudo PI_USER=pi PI_HOSTNAME=<your-hostname> ./setup_pi.sh

# The script will:
#   - Install Mosquitto MQTT broker (port 1883)
#   - Create Python venv, install Flask + paho-mqtt
#   - Copy main.py → ~/irrigation/
#   - Create systemd service: "irrigation"

# 5. Start the server:
sudo systemctl start irrigation

# 6. Check it's running:
sudo systemctl status irrigation
# Open in browser: http://<PI_IP>:5000

# 7. View logs:
journalctl -u irrigation -f

# 8. MQTT test:
mosquitto_sub -t 'sulama/#' -v
```

### 6b. ESP32-S3 Setup (Arduino IDE)

**Apply the same steps for all 4 ESPs.**

1. **Install Arduino IDE** (2.x recommended)

2. **Add ESP32 board support:**
   - File → Preferences → Additional Board Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board Manager → search "esp32" → install **"esp32 by Espressif Systems"**

3. **Install required libraries** (Tools → Library Manager):
   - **ArduinoJson** — Benoit Blanchon (v7.x)
   - **PubSubClient** — Nick O'Leary (v2.8+) — for all ESPs
   - **DHT sensor library** — Adafruit (ESP1 only)

4. **Board settings** (Tools menu):
   - Board: **ESP32S3 Dev Module**
   - USB CDC On Boot: **Enabled**
   - Flash Size: depending on your board (4MB / 8MB / 16MB)
   - Partition Scheme: **Default 4MB with spiffs**
   - Upload Speed: **921600**

5. **Edit WiFi and MQTT credentials** (in each .ino file):
   ```cpp
   const char* WIFI_SSID     = "YOUR_WIFI_SSID";           // ← your WiFi SSID
   const char* WIFI_PASSWORD = "";                           // ← your WiFi password
   const char* MQTT_BROKER   = "YOUR_PI_HOSTNAME.local";    // ← your Pi's IP address or hostname
   ```

6. **Upload to each ESP:**
   - ESP1: `esp1_gateway/esp1_gateway.ino` → Upload
   - ESP2: `esp2_valve8/esp2_valve8.ino` → Upload
   - ESP3: `esp3_valve4a/esp3_valve4a.ino` → Upload
   - ESP4: `esp4_valve4b/esp4_valve4b.ino` → Upload

7. **Check with Serial Monitor** (115200 baud):
   - You should see '✅ WiFi connected' and '✅ ESP{N} ready!' messages

### 6c. Physical Wiring Steps

1. **Connect relay module to ESP** (per Section 2 tables)
2. **Connect sensors to ESP1** (DHT pin GPIO 7, soil moisture GPIO 1)
3. **Connect relay outputs to solenoid valves / pump contactors**
4. **Power everything**: ESP → USB, relay VCC → 5V, sensors → 3.3V
5. **Connect common ground**: ESP GND + relay GND to same point

### 6d. Solenoid Valve Connection (relay output side)

Each relay has 3 terminals: **COM** (common), **NO** (normally open), **NC** (normally closed).

For irrigation valves (12V or 24V AC solenoid):
```
Relay COM ──── Solenoid wire 1
Relay NO  ──── Power supply (+)
               Power supply (−) ──── Solenoid wire 2
```

> Use **NO** (normally open) — so valves stay closed when power is cut (fail-safe).

---

## 7. Test Checklist

| # | Test | How to Check |
|---|------|--------------|
| 1 | Pi web interface loads | Open in browser: `http://<PI_IP>:5000` |
| 2 | MQTT broker is running | `mosquitto_sub -t 'sulama/#' -v` |
| 3 | ESPs connect to WiFi | IP address visible in Serial Monitor |
| 4 | ESPs connect to MQTT | Green ESP dots in web UI |
| 5 | Pump relay clicks | Press PUMP button, hear 3 clicks (10s, 4s, 4s apart) |
| 6 | Valve relay clicks | Press zone button, relay should click on correct ESP |
| 7 | City water safety | Open city water while pump is on — should be blocked |
| 8 | Timer works | Start a 1-min zone, check automatic shutoff |
| 9 | Sensor data incoming | Temperature/humidity/soil info in web UI |
| 10 | Schedule works | Add weekly schedule, verify it triggers at correct time |

---

## 8. Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| ESP cannot connect to WiFi | Wrong SSID/password | Check WiFi credentials in .ino file |
| Cannot connect to MQTT | Mosquitto not running or wrong IP | `sudo systemctl status mosquitto`, check IP/hostname |
| Relays turn ON at startup | Active-LOW relay module | Invert logic in code (see Section 4) |
| Web interface blank | Server crashed | Check errors with: `journalctl -u irrigation -f` |
| Valve doesn't open | Relay wiring wrong | Check COM/NO terminals, test with multimeter |
| Pump and city water both open | Should never happen (blocked by software) | Check server logs, press emergency stop |
| Sensor shows '--' | DHT not connected or wrong pin | Check connection to GPIO 7, verify library installed |
| ESP keeps rebooting | Insufficient power | Replace USB cable, try separate power supply |

---

## 9. File Structure

```
irrigation/
├── main.py                    # Pi web server (Flask + MQTT + scheduler)
├── esp1_pump.ino              # ESP1: Pump controller + sensors
├── esp2_valve8r.ino           # ESP2: 8-channel relay (valves 1-7 + city water)
├── esp3_valve4r.ino           # ESP3: 4-channel relay (valves 8-11)
├── esp4_valve4r.ino           # ESP4: 4-channel relay (valves 12-15)
├── configure.sh               # One-time setup: fill in credentials
├── setup_pi.sh                # Pi dependency installer
├── config.example.json        # Config template (copy → config.json)
├── WIRING.md                  # This file
└── .gitignore
```
