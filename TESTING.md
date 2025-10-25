# Testing Guide

## Pre-deployment Testing Checklist

Use this checklist before deploying to production.

### Hardware Testing

#### Phase 1: Power Test
- [ ] ESP32 boots successfully with 5V power
- [ ] Serial monitor shows boot messages
- [ ] No unusual restarts or crashes
- [ ] Power LED indicator is on

#### Phase 2: WiFi Connection Test
- [ ] ESP32 connects to WiFi within 10 seconds
- [ ] IP address is assigned
- [ ] IP address is displayed on serial monitor
- [ ] Connection remains stable for 5 minutes

#### Phase 3: Relay Test (No Valves)
- [ ] Relay 1 clicks when Zone 1 activated
- [ ] Relay 2 clicks when Zone 2 activated
- [ ] Relay 3 clicks when Zone 3 activated
- [ ] Relay 4 clicks when Zone 4 activated
- [ ] Relay LEDs indicate activation
- [ ] Relays turn off after duration expires

#### Phase 4: Valve Test (With Water)
- [ ] Zone 1 valve opens and water flows
- [ ] Zone 2 valve opens and water flows
- [ ] Zone 3 valve opens and water flows
- [ ] Zone 4 valve opens and water flows
- [ ] No leaks at connections
- [ ] All valves close properly
- [ ] No water drips after closing

### Software Testing

#### Web Interface Test
- [ ] Web page loads successfully
- [ ] All 4 zones are displayed
- [ ] System status shows correct information
- [ ] Current time is displayed
- [ ] WiFi status shows "Connected"
- [ ] IP address is correct

#### Manual Control Test
- [ ] Can start Zone 1 with custom duration
- [ ] Can start Zone 2 with custom duration
- [ ] Can start Zone 3 with custom duration
- [ ] Can start Zone 4 with custom duration
- [ ] Can stop any zone mid-operation
- [ ] Duration input accepts values 1-60
- [ ] Status updates in real-time (green/red indicator)
- [ ] Remaining time counts down correctly

#### Schedule Test
- [ ] Can open schedule panel for each zone
- [ ] Can enable/disable schedules
- [ ] Can set time for each schedule
- [ ] Can set duration for each schedule
- [ ] Schedule saves successfully
- [ ] Saved schedules persist after reboot
- [ ] Schedule triggers at correct time
- [ ] Multiple schedules work independently

#### API Test
```bash
# Test status endpoint
curl http://[ESP32-IP]/api/status
# Expected: JSON with system status

# Test start zone
curl -X POST http://[ESP32-IP]/api/zone/1/start \
  -H "Content-Type: application/json" \
  -d '{"duration": 60000}'
# Expected: {"success":true}

# Test stop zone
curl -X POST http://[ESP32-IP]/api/zone/1/stop
# Expected: {"success":true}

# Test schedule update
curl -X POST http://[ESP32-IP]/api/zone/1/schedule \
  -H "Content-Type: application/json" \
  -d '{"schedules":[{"enabled":true,"hour":6,"minute":0,"duration":600000}]}'
# Expected: {"success":true}
```

- [ ] Status API returns valid JSON
- [ ] Start API activates zone
- [ ] Stop API deactivates zone
- [ ] Schedule API saves configuration
- [ ] Invalid requests return error messages

### Safety Testing

#### Timeout Test
- [ ] Zone stops after maximum duration (1 hour)
- [ ] Manual stop works at any time
- [ ] System prevents duration > 1 hour

#### Crash Recovery Test
- [ ] System recovers from power loss
- [ ] Schedules persist after reboot
- [ ] All zones are OFF after restart
- [ ] No valves stuck open after crash

#### Concurrent Operation Test
- [ ] Can run multiple zones simultaneously
- [ ] Stopping one zone doesn't affect others
- [ ] Schedules don't interfere with manual control

### Performance Testing

#### Stress Test
- [ ] Run all zones for 1 hour each
- [ ] Check for memory leaks (monitor serial)
- [ ] Verify web interface remains responsive
- [ ] Check for any errors in logs

#### Long-term Test
- [ ] System runs for 24 hours without issues
- [ ] Scheduled operations execute correctly
- [ ] Web interface remains accessible
- [ ] No unexpected restarts

#### Network Test
- [ ] Web interface works from mobile phone
- [ ] Web interface works from tablet
- [ ] Web interface works from computer
- [ ] Multiple devices can access simultaneously
- [ ] System handles WiFi disconnect/reconnect

## Test Scenarios

### Scenario 1: Morning Watering Routine
**Setup:**
- Zone 1: 6:00 AM, 10 minutes (Lawn)
- Zone 2: 6:15 AM, 15 minutes (Garden)
- Zone 3: 6:35 AM, 20 minutes (Trees)
- Zone 4: Disabled

**Expected:**
1. Zone 1 starts at 6:00 AM
2. Zone 1 stops at 6:10 AM
3. Zone 2 starts at 6:15 AM
4. Zone 2 stops at 6:30 AM
5. Zone 3 starts at 6:35 AM
6. Zone 3 stops at 6:55 AM

**Test:**
- [ ] All zones activated at correct times
- [ ] All durations were accurate
- [ ] No zones overlapped unexpectedly
- [ ] Web interface showed correct status

### Scenario 2: Emergency Stop
**Setup:**
- Start Zone 1 for 30 minutes

**Actions:**
1. Wait 5 minutes
2. Click "Stop" button

**Expected:**
- [ ] Zone 1 stops immediately
- [ ] Valve closes completely
- [ ] No water continues to flow
- [ ] Web interface updates status

### Scenario 3: Schedule Override
**Setup:**
- Zone 1 scheduled for 6:00 AM
- Manually start Zone 1 at 5:55 AM

**Expected:**
- [ ] Manual operation works
- [ ] Schedule doesn't start at 6:00 AM (already running)
- [ ] Manual stop works normally

### Scenario 4: Power Failure Recovery
**Actions:**
1. Start Zone 1 for 30 minutes
2. Disconnect power after 10 minutes
3. Reconnect power

**Expected:**
- [ ] System reboots successfully
- [ ] All zones are OFF
- [ ] Zone 1 is not running
- [ ] Schedules are still saved
- [ ] Web interface is accessible

## Performance Benchmarks

### Response Time
- Web page load: < 2 seconds
- API response: < 200ms
- Zone activation: < 100ms
- Schedule check: < 50ms

### Memory Usage
Check via serial monitor during operation:
```cpp
// Add to loop() for monitoring
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
```

Expected:
- Free heap: > 100KB during normal operation
- No continuous decrease (memory leak)

### Network Stability
- WiFi reconnect: < 10 seconds
- No disconnects during normal operation
- Web server handles 10+ requests/second

## Debugging Tips

### Enable Verbose Logging
Add to `setup()`:
```cpp
Serial.setDebugOutput(true);
```

### Monitor Zone Status
Add to `loop()`:
```cpp
static unsigned long lastDebug = 0;
if (millis() - lastDebug > 5000) {
  lastDebug = millis();
  for (int i = 0; i < NUM_ZONES; i++) {
    Serial.printf("Zone %d: %s, Remaining: %lu\n", 
                  i+1, 
                  zones[i]->getStatus() ? "ON" : "OFF",
                  zones[i]->getRemainingTime());
  }
}
```

### Check Memory Leaks
Add to `loop()`:
```cpp
static unsigned long lastMem = 0;
if (millis() - lastMem > 10000) {
  lastMem = millis();
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
}
```

## Known Limitations

1. **Maximum 4 zones** - Hardware limited by available GPIO pins
2. **60-minute maximum** via web interface (safety)
3. **3 schedules per zone** - Balance between features and memory
4. **No moisture sensors** - Future enhancement
5. **No weather integration** - Future enhancement

## Troubleshooting Test Failures

### Web Interface Not Loading
1. Verify IP address is correct
2. Check WiFi connection
3. Try different browser
4. Clear browser cache
5. Check serial monitor for errors

### Zone Not Activating
1. Check relay connection
2. Verify power supply (5V to relay)
3. Test relay manually with jumper wire
4. Check GPIO pin number in config.h
5. Verify valve power supply (12V)

### Schedule Not Triggering
1. Check time synchronization (needs internet)
2. Verify schedule is enabled
3. Check schedule time format (24-hour)
4. Look for errors in serial monitor
5. Ensure zone is not already running

### System Crashes/Resets
1. Check power supply capacity
2. Add decoupling capacitors
3. Separate ESP32 and relay power
4. Check for memory leaks
5. Review serial monitor before crash

## Test Report Template

```
Test Date: ___________
Tester: ___________
Hardware Version: ___________
Software Version: ___________

Hardware Tests:
[ ] Power Test - PASS/FAIL
[ ] WiFi Test - PASS/FAIL
[ ] Relay Test - PASS/FAIL
[ ] Valve Test - PASS/FAIL

Software Tests:
[ ] Web Interface - PASS/FAIL
[ ] Manual Control - PASS/FAIL
[ ] Schedule - PASS/FAIL
[ ] API - PASS/FAIL

Safety Tests:
[ ] Timeout - PASS/FAIL
[ ] Crash Recovery - PASS/FAIL

Performance Tests:
[ ] Response Time - PASS/FAIL
[ ] Memory Usage - PASS/FAIL
[ ] Long-term Stability - PASS/FAIL

Issues Found:
1. ___________
2. ___________
3. ___________

Overall Result: PASS/FAIL

Notes:
___________
___________
```

## Pre-Production Checklist

Before leaving the system unattended:

- [ ] All hardware tests passed
- [ ] All software tests passed
- [ ] All safety tests passed
- [ ] 24-hour stability test completed
- [ ] Schedules configured correctly
- [ ] Emergency shutoff valve installed
- [ ] Waterproof enclosure secured
- [ ] User instructed on operation
- [ ] Contact information for support provided
- [ ] Maintenance schedule established

---

**Remember**: Safety first! Never deploy a system that hasn't passed all critical tests.
