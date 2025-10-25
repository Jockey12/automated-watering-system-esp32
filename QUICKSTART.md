# Quick Start Guide

## For First-Time Users

This guide will help you get your automated watering system up and running in under 30 minutes.

### Step 1: Hardware Assembly (5 minutes)

**What you need:**
- ESP32-S3-DevKitC-1 board
- 4-channel relay module
- 4 solenoid valves (12V)
- Power supplies (5V and 12V)
- Jumper wires

**Quick Connect:**
```
ESP32 GPIO 1 → Relay IN1
ESP32 GPIO 2 → Relay IN2  
ESP32 GPIO 3 → Relay IN3
ESP32 GPIO 4 → Relay IN4
ESP32 GND → Relay GND
ESP32 5V → Relay VCC

Relay COM → 12V+
Relay NO → Valve +
Valve - → 12V GND
```

> ⚠️ **Safety First**: Double-check all connections before powering on!

### Step 2: Software Setup (10 minutes)

1. **Install PlatformIO**
   ```bash
   pip install platformio
   ```

2. **Clone and Configure**
   ```bash
   git clone https://github.com/Jockey12/automated-watering-system-esp32.git
   cd automated-watering-system-esp32
   ```

3. **Edit WiFi Settings**
   
   Open `src/config.h` and update:
   ```cpp
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourPassword"
   ```

4. **Upload to ESP32**
   ```bash
   platformio run --target upload
   ```

### Step 3: First Access (5 minutes)

1. **Find Your IP Address**
   
   Open serial monitor:
   ```bash
   platformio device monitor
   ```
   
   Look for a line like: `IP address: 192.168.1.100`

2. **Open Web Interface**
   
   Open your browser and go to: `http://192.168.1.100`
   
   You should see a beautiful purple interface with 4 zones!

### Step 4: Test Your Zones (5 minutes)

1. **Manual Test**
   - Set duration to `1` minute
   - Click **Start** on Zone 1
   - Listen for relay clicking
   - Watch for water flow
   - Verify automatic shutoff after 1 minute

2. **Test All Zones**
   - Repeat for zones 2, 3, and 4
   - Ensure each valve works properly

### Step 5: Set Up Schedules (5 minutes)

1. **Open Schedule Panel**
   - Click **Schedule** button on any zone
   - A panel will expand with 3 schedule slots

2. **Create First Schedule**
   - ✓ Enable schedule
   - Set time: `06:00` (6 AM)
   - Set duration: `10` minutes
   - Click **Save Schedules**

3. **Create More Schedules**
   - You can have up to 3 schedules per zone
   - Each zone is independent
   - Example: Morning (6 AM), Evening (6 PM)

## Common Issues & Quick Fixes

### ❌ Can't Connect to WiFi
- Check SSID and password spelling
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Move closer to router

### ❌ Web Page Won't Load
- Verify IP address from serial monitor
- Try accessing from same WiFi network
- Ping the IP: `ping 192.168.1.100`

### ❌ Relay Clicks But No Water
- Check 12V power supply to valves
- Verify valve connections
- Test valve with multimeter
- Ensure water source is on!

### ❌ Schedule Doesn't Work
- Check that time is synchronized (needs internet)
- Verify schedule is enabled (checkbox)
- Check serial monitor for schedule triggers

## Usage Tips

### 💡 Best Practices

1. **Start with short durations** (1-2 minutes) for testing
2. **Set different times for each zone** to avoid power spikes
3. **Use schedules for daily automation**, manual for adjustments
4. **Check system weekly** for leaks or issues

### 💡 Optimal Watering Times

- **Early Morning (5-7 AM)**: Best time, less evaporation
- **Evening (6-8 PM)**: Good alternative
- **Avoid Midday**: Too much evaporation

### 💡 Duration Guidelines

| Plant Type | Duration | Frequency |
|------------|----------|-----------|
| Lawn | 15-20 min | Daily |
| Vegetables | 10-15 min | Daily |
| Shrubs | 15-30 min | 2-3x week |
| Trees | 30-60 min | Weekly |

> Note: Adjust based on soil type, weather, and season

## Next Steps

Once your system is running:

1. **Monitor Performance**
   - Check soil moisture manually for first week
   - Adjust durations as needed
   - Ensure no over/under watering

2. **Optimize Schedules**
   - Adjust based on weather
   - Reduce watering if rain expected
   - Increase during hot weather

3. **Maintenance**
   - Check for leaks weekly
   - Clean valve filters monthly
   - Test all zones regularly

## Advanced Features

### API Access

You can control the system programmatically:

```bash
# Start Zone 1 for 10 minutes
curl -X POST http://192.168.1.100/api/zone/1/start \
  -H "Content-Type: application/json" \
  -d '{"duration": 600000}'

# Stop Zone 1
curl -X POST http://192.168.1.100/api/zone/1/stop

# Get system status
curl http://192.168.1.100/api/status
```

### Home Automation Integration

The API endpoints work with:
- Home Assistant
- Node-RED
- IFTTT
- Any system that supports HTTP requests

### Timezone Configuration

To change timezone, edit `src/main.cpp`:

```cpp
const long gmtOffset_sec = -28800;  // PST (UTC-8)
const int daylightOffset_sec = 3600;  // DST (+1 hour)
```

## Getting Help

- **Documentation**: See README.md for detailed info
- **Hardware Guide**: See HARDWARE_SETUP.md for wiring
- **Issues**: Open an issue on GitHub
- **Updates**: Check repository for new features

## Safety Reminders

- ⚠️ Never leave system unattended for extended periods initially
- ⚠️ Install emergency shutoff valve on water line
- ⚠️ Use weatherproof enclosure for outdoor installation  
- ⚠️ Check local regulations for automated watering
- ⚠️ Don't exceed valve/relay ratings
- ⚠️ Ensure proper electrical safety

---

**Congratulations!** 🎉 Your automated watering system is now operational!

For more detailed information, refer to the complete README.md and HARDWARE_SETUP.md guides.
