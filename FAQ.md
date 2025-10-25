# Frequently Asked Questions (FAQ)

## General Questions

### Q: What hardware do I need?
**A:** You need:
- ESP32-S3-DevKitC-1 (N16R8 variant recommended)
- 4-channel relay module (5V)
- 4 solenoid valves (12V typically)
- Power supplies (5V for ESP32/relays, 12V for valves)
- Jumper wires and enclosure

See [HARDWARE_SETUP.md](HARDWARE_SETUP.md) for details.

### Q: Can I use a different ESP32 board?
**A:** The code is designed for ESP32-S3, but can be adapted for other ESP32 variants (ESP32, ESP32-C3, etc.) with modifications to both the platformio.ini file (board selection) and config.h (pin definitions). Different ESP32 variants have different available GPIO pins and capabilities.

### Q: Can I use more than 4 zones?
**A:** Currently v1.0.0 supports 4 zones. Support for more zones is planned for v2.0.0. You can modify the code to add more zones by changing `NUM_ZONES` in config.h and adding more GPIO pins.

### Q: Does this work with 110V/220V AC valves?
**A:** The relays can switch AC loads, but this project is designed for 12V DC solenoid valves. If using AC valves, ensure your relays are rated appropriately and take proper safety precautions. **AC wiring should only be done by qualified electricians.**

### Q: How much does this cost to build?
**A:** Approximate costs:
- ESP32-S3: $8-15
- 4-channel relay module: $5-10
- 4 solenoid valves: $20-40 ($5-10 each)
- Power supplies: $10-15
- Miscellaneous (wires, enclosure): $10-20
- **Total: ~$50-100 USD**

### Q: Can this system work offline?
**A:** Yes, manual control and schedules work offline. However, initial time synchronization requires internet connection. Once synced, the system maintains time using the ESP32's internal clock.

## Installation & Setup

### Q: How do I upload the code to ESP32?
**A:** 
1. Install PlatformIO
2. Connect ESP32 via USB
3. Run `platformio run --target upload`
4. See [QUICKSTART.md](QUICKSTART.md) for detailed steps

### Q: The code won't compile. What's wrong?
**A:** Common issues:
- PlatformIO not installed correctly
- Wrong board selected
- Missing libraries (PlatformIO should auto-install)
- Network issues preventing library download
- Check that you're using ESP32 platform, not Arduino AVR

### Q: How do I find my ESP32's IP address?
**A:** 
- Open serial monitor: `platformio device monitor`
- Look for line showing "IP address: 192.168.x.x"
- Alternatively, check your router's DHCP client list

### Q: Can I use a static IP address?
**A:** Yes, modify the WiFi setup in main.cpp:
```cpp
WiFi.config(IPAddress(192, 168, 1, 100), 
            IPAddress(192, 168, 1, 1), 
            IPAddress(255, 255, 255, 0));
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
```

### Q: How do I change the timezone?
**A:** Edit main.cpp and modify:
```cpp
const long gmtOffset_sec = -28800;  // Your timezone offset
const int daylightOffset_sec = 3600;  // DST offset if applicable
```

## Hardware Issues

### Q: My relay clicks but the valve doesn't open. Why?
**A:** Check:
1. Valve has 12V power
2. Valve polarity is correct (try reversing wires)
3. Valve is not clogged or stuck
4. Water pressure is adequate
5. Relay contacts are actually switching (test with multimeter)

### Q: Can I use active-LOW relays instead of active-HIGH?
**A:** Yes, you need to modify WateringZone.cpp:
```cpp
// For active-LOW relays:
void WateringZone::start(uint32_t durationMs) {
  digitalWrite(pin, LOW);  // Change HIGH to LOW
}

void WateringZone::stop() {
  digitalWrite(pin, HIGH);  // Change LOW to HIGH
}
```

### Q: My ESP32 resets when I activate a relay. What's wrong?
**A:** This indicates power supply issues:
- Use separate power supplies for ESP32 and relays
- Ensure common ground connection
- Add bulk capacitor (1000µF) near ESP32 power pins
- Check power supply can handle current draw

### Q: Which GPIO pins can I use on ESP32-S3?
**A:** Safe GPIO pins: 1-18, 21, 33-48
Avoid: 0, 19-20 (USB), 26-32 (PSRAM/Flash), strapping pins during boot

### Q: Can I power everything from USB?
**A:** Only for testing without valves. USB provides ~500mA, which is insufficient for relays + valves. Use proper power supplies for production.

## Web Interface Issues

### Q: Web page won't load. What should I check?
**A:** Troubleshooting steps:
1. Verify ESP32 is connected to WiFi (check serial monitor)
2. Confirm you're on the same WiFi network
3. Try accessing IP address directly (not hostname)
4. Check browser console for errors (F12)
5. Try different browser
6. Clear browser cache

### Q: Web interface is slow or unresponsive
**A:** Common causes:
- Weak WiFi signal - move closer or use WiFi extender
- Multiple devices accessing simultaneously
- ESP32 overheating - ensure adequate cooling
- Check serial monitor for errors or crashes

### Q: Status doesn't update in real-time
**A:** The interface polls every 2 seconds. If not updating:
- Check browser console for JavaScript errors
- Verify /api/status endpoint works (visit directly)
- Check WiFi stability

### Q: Can I change the web interface appearance?
**A:** Yes, edit the HTML/CSS in main.cpp. The entire interface is embedded in the firmware. Search for `server.on("/", HTTP_GET` to find the HTML.

## Scheduling Issues

### Q: My schedules don't trigger. Why?
**A:** Check:
1. Schedule is enabled (checkbox)
2. Time is correct (check system time on web interface)
3. Time zone is configured correctly
4. ESP32 has internet for NTP sync (at least once)
5. Serial monitor shows schedule checks

### Q: Can I have multiple schedules per zone?
**A:** Yes, up to 3 schedules per zone. Each schedule is independent.

### Q: What happens if two zones are scheduled at the same time?
**A:** Both will run simultaneously if their schedules trigger. Consider staggering start times to avoid power draw spikes.

### Q: Can I set schedules in 12-hour format?
**A:** The time input uses 24-hour format. 6 AM = 06:00, 6 PM = 18:00.

### Q: Do schedules persist after power loss?
**A:** Yes, schedules are stored in NVS (non-volatile storage) and survive power cycles and reboots.

### Q: How accurate is the scheduling?
**A:** Schedules check every minute. Accuracy depends on NTP sync. Typical accuracy is within 1-2 seconds.

## Safety & Operation

### Q: What happens if I lose power during watering?
**A:** All zones turn OFF when power is restored. Schedules remain saved. This is a safety feature.

### Q: Can a zone stay on forever if something goes wrong?
**A:** No. There's a maximum duration limit (default 1 hour). Zones automatically shut off after this time even if something fails.

### Q: Is it safe to leave this running unattended?
**A:** After thorough testing, yes. However:
- Test extensively first
- Install emergency shutoff valve
- Use waterproof enclosures
- Check regularly for first few weeks
- Follow all safety guidelines

### Q: What if my WiFi goes down?
**A:** Schedules continue to work (using internal clock). Web interface becomes inaccessible until WiFi returns. Manual override not available during WiFi outage.

### Q: Can I water multiple zones simultaneously?
**A:** Yes, zones are independent. However, consider:
- Power supply capacity
- Water pressure availability
- Total current draw on relays

## Integration & Advanced

### Q: Can I integrate with Home Assistant?
**A:** Not directly in v1.0.0. Use REST API to create custom integrations. MQTT support planned for v1.3.0.

### Q: Can I control this with Alexa/Google Home?
**A:** Not directly in v1.0.0. Voice control planned for v1.4.0. Currently, you can use IFTTT webhooks with the REST API.

### Q: How do I use the REST API?
**A:** See [README.md](README.md#api-endpoints) for API documentation. Use curl or any HTTP client:
```bash
# Get status
curl http://192.168.1.100/api/status

# Start zone
curl -X POST http://192.168.1.100/api/zone/1/start \
  -H "Content-Type: application/json" \
  -d '{"duration": 600000}'
```

### Q: Can I add soil moisture sensors?
**A:** Not in v1.0.0. Sensor integration planned for v1.1.0. You can modify the code to add sensor support.

### Q: Is there a mobile app?
**A:** No native app in v1.0.0. The web interface is mobile-responsive. Native apps planned for v1.4.0.

### Q: Can I monitor water usage?
**A:** Not in v1.0.0. Water flow meter support planned for v1.2.0.

### Q: How do I backup my configuration?
**A:** Schedules are stored in NVS on the ESP32. To backup:
1. Save your current schedules (screenshot web interface)
2. Note pin configurations from config.h
3. Configuration can be re-entered via web interface after reflashing

## Performance & Limits

### Q: How many devices can access the web interface simultaneously?
**A:** Tested with 5+ concurrent connections without issues. The async web server handles multiple clients efficiently.

### Q: What's the maximum watering duration?
**A:** Default maximum is 1 hour (configurable in config.h). Web interface limits to 60 minutes. This is a safety feature.

### Q: How long can zones run continuously?
**A:** Technically indefinitely (within max duration), but consider:
- Solenoid valve duty cycle ratings
- Power supply heat dissipation
- Water pressure and supply

### Q: Does this consume a lot of power?
**A:** Power consumption:
- ESP32-S3: ~200mA (active)
- Relay module: ~60-80mA (idle), +70mA per active relay
- Solenoid valve: ~300-500mA each when active
- Total idle: ~300mA @ 5V = ~1.5W
- Total active (all 4 zones): ~2.5-3.5A @ 12V = ~30-40W

### Q: Will this overheat in an outdoor enclosure?
**A:** ESP32-S3 can operate 0-85°C. Use:
- Ventilated enclosure
- Heat sinks on ESP32 and relays
- White/reflective enclosure
- Shade if possible
- Consider active cooling for extreme climates

## Troubleshooting

### Q: Serial monitor shows "WiFi connection failed"
**A:** 
- Double-check SSID and password in config.h
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check WiFi router is on and broadcasting
- Move ESP32 closer to router
- Check for special characters in password

### Q: Serial monitor shows "Failed to obtain time"
**A:**
- Requires internet connection for NTP
- Check router allows NTP traffic (port 123)
- Firewall may be blocking NTP
- Time zone offset may be incorrect

### Q: Serial monitor shows "Brownout detector was triggered"
**A:**
- Power supply voltage dropped too low
- Use better quality power supply
- Check power supply current rating
- Add decoupling capacitors

### Q: Web server started but can't connect
**A:**
- Check IP address is correct
- Verify on same network
- Try ping: `ping 192.168.1.100`
- Check firewall settings
- Try different browser

### Q: Zone runs longer than scheduled duration
**A:**
- Possible millis() overflow (rare, 49 days uptime)
- Reset ESP32 to fix
- Check serial monitor for errors

### Q: Schedules are 1 hour off
**A:**
- Daylight saving time issue
- Adjust `daylightOffset_sec` in main.cpp
- Or adjust schedule times by 1 hour

## Maintenance

### Q: How often should I test the system?
**A:** 
- Daily: First week after installation
- Weekly: First month
- Monthly: After that
- Before/after: Any extended absence

### Q: Do I need to update the firmware?
**A:** Not required, but recommended for:
- Bug fixes
- New features
- Security updates
Check GitHub for releases

### Q: How do I reset to factory defaults?
**A:** Flash new firmware or use NVS erase:
```bash
platformio run --target erase
platformio run --target upload
```

### Q: Can I repair a failed solenoid valve?
**A:** Most valves have replaceable parts (diaphragm, spring). Check manufacturer documentation. Often it's cheaper to replace.

## Contributing

### Q: How can I contribute?
**A:** See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines:
- Report bugs
- Suggest features
- Submit code improvements
- Improve documentation
- Test and provide feedback

### Q: I found a bug. Where do I report it?
**A:** Open an issue on GitHub with:
- Description of the bug
- Steps to reproduce
- Expected vs actual behavior
- Hardware/software versions
- Serial monitor output

### Q: Can I fork this project for commercial use?
**A:** Yes, project is MIT licensed. You're free to use, modify, and distribute commercially. Attribution appreciated but not required.

## Future Features

### Q: When will [feature X] be available?
**A:** Check [ROADMAP.md](ROADMAP.md) for planned features and estimated timelines. Timelines are estimates and may change.

### Q: Can I request a feature?
**A:** Yes! Open a feature request on GitHub. Popular features get priority.

### Q: Will this support other microcontrollers?
**A:** Currently ESP32 only. Other platforms (RP2040, STM32) possible but not planned. Community contributions welcome!

---

## Still Have Questions?

- Check [README.md](README.md) for detailed documentation
- See [QUICKSTART.md](QUICKSTART.md) for setup instructions
- Read [HARDWARE_SETUP.md](HARDWARE_SETUP.md) for wiring details
- Review [TESTING.md](TESTING.md) for testing procedures
- Open an issue on GitHub if question not answered

**Last Updated**: 2025-10-25
