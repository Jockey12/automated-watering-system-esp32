# Automated Irrigation System

WiFi-connected irrigation controller using a **Raspberry Pi** as the hub and **4 × ESP32-S3** boards as field controllers. Control up to 15 valve zones + a pump via a web UI and weekly schedule.

```
Raspberry Pi (Flask + MQTT broker)
    │ WiFi
    ├── ESP1 — Pump controller (3-breaker sequence)
    ├── ESP2 — 8-channel valve board (zones 1–7 + city water)
    ├── ESP3 — 4-channel valve board (zones 8–11)
    └── ESP4 — 4-channel valve board (zones 12–15) + soil/temp sensors
```

---

## Hardware

| Component | Qty | Notes |
|-----------|-----|-------|
| Raspberry Pi 3 / 4 | 1 | Runs the web server and MQTT broker |
| ESP32-S3 Dev Module | 4 | One per relay board |
| 4-channel relay module | 3 | ESP1 (pump), ESP3, ESP4 |
| 8-channel relay module | 1 | ESP2 |
| Solenoid valves (12 V or 24 V AC) | up to 15 | Your irrigation valves |
| Soil moisture sensor (analog) | 1 | Connected to ESP4 GPIO 1 |
| NTC thermistor (10 kΩ) | 1 | Connected to ESP4 GPIO 2 |

See [`src/WIRING.md`](src/WIRING.md) for full pin tables and safety rules.

---

## Quick Start

### 1. Clone and configure

```bash
git clone https://github.com/YOUR_USERNAME/YOUR_REPO.git
cd YOUR_REPO/src
chmod +x configure.sh
./configure.sh
```

The script asks for:
- **Garden name** — shown in Serial Monitor output
- **WiFi SSID & password** — written into each `.ino` file
- **Pi hostname** — e.g. `irrigation-pi` (reachable as `irrigation-pi.local`)
- **Pi Linux username** — e.g. `pi`
- **Timezone** — IANA format, e.g. `America/New_York`, `Europe/London`

It creates `config.json` (read by the Python server) and patches the `.ino` files in-place.

> ⚠️ **Never commit your `.ino` files or `config.json` after running configure.sh** — they contain your WiFi password.

---

### 2. Flash the ESP32 boards (Arduino IDE)

**One-time setup:**
1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Add ESP32 board support — Preferences → Additional Board Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board Manager → search **esp32 by Espressif Systems** → Install
4. Tools → Library Manager → install: **ArduinoJson**, **PubSubClient**, **Adafruit NeoPixel**

**Board settings** (Tools menu):
- Board: `ESP32S3 Dev Module`
- USB CDC On Boot: `Enabled`
- Upload Speed: `921600`

**Upload order:**
| File | Target |
|------|--------|
| `esp1_pump.ino` | ESP1 |
| `esp2_valve8r.ino` | ESP2 |
| `esp3_valve4r.ino` | ESP3 |
| `esp4_valve4r.ino` | ESP4 |

Open Serial Monitor at **115200 baud** to verify each board connects to WiFi and MQTT.

---

### 3. Set up the Raspberry Pi

```bash
# Copy project to Pi
scp -r . pi@irrigation-pi.local:~/irrigation/

# SSH in and run the setup script
ssh pi@irrigation-pi.local
cd ~/irrigation
sudo PI_USER=pi PI_HOSTNAME=irrigation-pi ./setup_pi.sh

# Start the server
sudo systemctl start irrigation

# Open the web UI
# http://irrigation-pi.local:5000
```

The setup script installs **Mosquitto** (MQTT broker), **avahi-daemon** (mDNS), creates a Python virtualenv, and registers an **`irrigation` systemd service** that starts on boot.

---

## Useful Commands

```bash
# Server logs (live)
journalctl -u irrigation -f

# Restart server
sudo systemctl restart irrigation

# Watch all MQTT traffic
mosquitto_sub -t 'sulama/#' -v

# Send a manual command (valve 3 open)
mosquitto_pub -t sulama/esp2/komut -m '{"cmd":"VALF_AC","valf":3}'

# Emergency stop all
mosquitto_pub -t sulama/tum/komut -m '{"cmd":"TUMU_KAPAT"}'
```

---

## Relay polarity

Code defaults to **active-HIGH** relays (`HIGH` = relay ON).  
If your relays activate on `LOW` (common on cheap Chinese boards — they stay ON at startup), swap the `HIGH`/`LOW` in each `.ino` file. See [`src/WIRING.md § Relay Polarity`](src/WIRING.md#4-relay-polarity-note) for details.

---

## License

See [LICENSE](LICENSE).
