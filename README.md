# Automated Watering System ESP32-S3

An automated garden watering system using ESP32-S3 N16R8 microcontroller with web-based control interface and scheduled watering capabilities.

## Features

- 🌱 **Multi-Zone Control**: Manage up to 4 independent watering zones
- 🌐 **Web Interface**: Beautiful, responsive web UI for system control
- ⏰ **Automated Scheduling**: Set up to 3 scheduled watering times per zone
- 🔒 **Safety Features**: Maximum duration limits and timeout protection
- 💾 **Persistent Storage**: Schedules and settings saved to NVS
- 📱 **Mobile Friendly**: Responsive design works on phones and tablets
- 🕐 **NTP Time Sync**: Automatic time synchronization for accurate scheduling

## Hardware Requirements

### Main Components
- **ESP32-S3-DevKitC-1** (N16R8 - 16MB Flash, 8MB PSRAM)
- **4x Relay Module** (5V, optocoupler isolated recommended)
- **4x Solenoid Valves** (12V DC or as required)
- **Power Supply**: 
  - 5V for ESP32 and relays
  - 12V for solenoid valves (or voltage matching your valves)
- **Water source** with adequate pressure

### Pin Configuration
Default pin assignments (can be modified in `src/config.h`):
- Zone 1: GPIO 1
- Zone 2: GPIO 2
- Zone 3: GPIO 3
- Zone 4: GPIO 4

**Note**: Ensure the selected pins are available and not used for critical ESP32-S3 functions (avoid strapping pins during boot).

## Software Setup

### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- USB cable for programming
- WiFi network credentials

### Installation Steps

1. **Clone the repository**
   ```bash
   git clone https://github.com/Jockey12/automated-watering-system-esp32.git
   cd automated-watering-system-esp32
   ```

2. **Configure WiFi**
   
   Edit `src/config.h` and update your WiFi credentials:
   ```cpp
   #define WIFI_SSID "YourWiFiSSID"
   #define WIFI_PASSWORD "YourWiFiPassword"
   ```

3. **Adjust Hardware Configuration** (if needed)
   
   In `src/config.h`, modify pin assignments to match your wiring:
   ```cpp
   #define ZONE1_PIN 1
   #define ZONE2_PIN 2
   #define ZONE3_PIN 3
   #define ZONE4_PIN 4
   ```

4. **Build and Upload**
   ```bash
   platformio run --target upload
   ```

5. **Monitor Serial Output**
   ```bash
   platformio device monitor
   ```
   
   The serial monitor will display:
   - WiFi connection status
   - IP address assigned
   - System initialization messages

## Usage

### Web Interface

1. **Access the Interface**
   - Open a web browser
   - Navigate to `http://[ESP32-IP-ADDRESS]`
   - The IP address is shown in the serial monitor on boot

2. **Manual Control**
   - Each zone has Start/Stop buttons
   - Set duration in minutes (1-60)
   - Click "Start" to begin watering
   - Click "Stop" to halt watering immediately

3. **Schedule Setup**
   - Click "Schedule" button for any zone
   - Enable up to 3 scheduled watering times per zone
   - Set time (24-hour format) and duration
   - Click "Save Schedules" to persist settings

### Features Overview

#### Zone Status
- **Green indicator**: Zone is actively watering
- **Red indicator**: Zone is inactive
- **Remaining time**: Shows countdown when active

#### Safety Features
- Maximum watering duration: 1 hour (configurable)
- Automatic shutoff when duration expires
- Only one schedule triggers at a time per zone

#### Time Synchronization
- Automatic NTP time sync on boot
- Requires internet connection
- Falls back gracefully if sync fails

## Hardware Assembly

### Wiring Diagram

```
ESP32-S3           Relay Module        Solenoid Valve
--------           ------------        --------------
GPIO 1    ------>  IN1        NO  ---> Valve 1 (+)
GPIO 2    ------>  IN2        NO  ---> Valve 2 (+)
GPIO 3    ------>  IN3        NO  ---> Valve 3 (+)
GPIO 4    ------>  IN4        NO  ---> Valve 4 (+)
GND       ------>  GND        COM ---> 12V (+)
5V        ------>  VCC        
                              
Valve (-) -----> 12V GND (Common)
```

### Safety Considerations

⚠️ **Important Safety Notes**:
- Use proper insulation for outdoor installations
- Ensure waterproof enclosures for electronics
- Use appropriate voltage regulators
- Add fuses for overcurrent protection
- Test thoroughly before leaving unattended
- Verify relay ratings match solenoid valve current draw

## Configuration Options

### In `config.h`:

```cpp
// WiFi Settings
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// GPIO Pins
#define ZONE1_PIN 1
#define ZONE2_PIN 2
#define ZONE3_PIN 3
#define ZONE4_PIN 4

// Number of zones (1-4)
#define NUM_ZONES 4

// Safety timeout (ms)
#define MAX_WATERING_DURATION 3600000  // 1 hour

// Default duration (ms)
#define DEFAULT_WATERING_DURATION 600000  // 10 minutes
```

## Troubleshooting

### WiFi Connection Issues
- Verify SSID and password in `config.h`
- Check WiFi signal strength at installation location
- Ensure 2.4GHz WiFi is enabled (ESP32 doesn't support 5GHz)

### Web Interface Not Loading
- Verify ESP32 IP address from serial monitor
- Check firewall settings
- Ensure device and ESP32 are on same network
- Try accessing via IP address directly

### Zones Not Activating
- Check relay module power supply (5V)
- Verify GPIO pin connections
- Test relay module independently
- Check solenoid valve power supply (12V)
- Verify valve voltage matches power supply

### Time/Schedule Issues
- Ensure internet connection for NTP sync
- Check timezone settings (adjust `gmtOffset_sec` in main.cpp)
- Verify schedules are saved (check serial output)

### Serial Monitor Shows Errors
- Common issues and solutions:
  - "WiFi connection failed": Check credentials
  - "Failed to obtain time": Check internet connection
  - Zone issues: Verify pin assignments

## API Endpoints

For custom integrations:

### GET `/api/status`
Returns system status including all zones, WiFi, time, and schedules.

### POST `/api/zone/{id}/start`
Start a zone. Body: `{"duration": milliseconds}`

### POST `/api/zone/{id}/stop`
Stop a zone immediately.

### POST `/api/zone/{id}/schedule`
Update zone schedules. Body:
```json
{
  "schedules": [
    {"enabled": true, "hour": 6, "minute": 0, "duration": 600000},
    {"enabled": false, "hour": 18, "minute": 0, "duration": 300000},
    {"enabled": false, "hour": 0, "minute": 0, "duration": 600000}
  ]
}
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## License

This project is licensed under the terms specified in the LICENSE file.

## Future Enhancements

Potential features for future versions:
- Soil moisture sensor integration
- Weather API integration (skip watering if rain detected)
- Mobile app
- MQTT support for home automation integration
- Water flow meters
- Multiple schedule profiles (summer/winter)
- Email/push notifications

## Support

For issues, questions, or contributions, please visit:
https://github.com/Jockey12/automated-watering-system-esp32