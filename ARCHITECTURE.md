# System Architecture

## Overview

The Automated Watering System consists of three main layers: Hardware, Firmware, and User Interface.

```
┌─────────────────────────────────────────────────────────────┐
│                      USER INTERFACE                         │
├─────────────────────────────────────────────────────────────┤
│  Web Browser                                                │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │  Zone 1  │  │  Zone 2  │  │  Zone 3  │  │  Zone 4  │   │
│  │  Start   │  │  Start   │  │  Start   │  │  Start   │   │
│  │  Stop    │  │  Stop    │  │  Stop    │  │  Stop    │   │
│  │  Schedule│  │  Schedule│  │  Schedule│  │  Schedule│   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
│                                                             │
│  Status Bar: WiFi, Time, IP Address                        │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ HTTP/JSON
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                     FIRMWARE LAYER                          │
├─────────────────────────────────────────────────────────────┤
│  ESP32-S3 (main.cpp)                                        │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Web Server (AsyncWebServer)                        │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐          │   │
│  │  │GET /     │  │GET /api/ │  │POST /api/│          │   │
│  │  │          │  │status    │  │zone/*/   │          │   │
│  │  └──────────┘  └──────────┘  └──────────┘          │   │
│  └─────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Core Logic                                         │   │
│  │  ┌────────────┐  ┌────────────┐  ┌──────────────┐  │   │
│  │  │ WiFi Mgr   │  │ NTP Time   │  │ NVS Storage  │  │   │
│  │  └────────────┘  └────────────┘  └──────────────┘  │   │
│  └─────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Zone Management (WateringZone.cpp)                │   │
│  │  ┌───────┐  ┌───────┐  ┌───────┐  ┌───────┐       │   │
│  │  │Zone 1 │  │Zone 2 │  │Zone 3 │  │Zone 4 │       │   │
│  │  │       │  │       │  │       │  │       │       │   │
│  │  │Active │  │Active │  │Active │  │Active │       │   │
│  │  │Timer  │  │Timer  │  │Timer  │  │Timer  │       │   │
│  │  │Sched  │  │Sched  │  │Sched  │  │Sched  │       │   │
│  │  └───────┘  └───────┘  └───────┘  └───────┘       │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  GPIO Control                                               │
│  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐                   │
│  │GPIO 1│  │GPIO 2│  │GPIO 3│  │GPIO 4│                   │
│  └───┬──┘  └───┬──┘  └───┬──┘  └───┬──┘                   │
└──────┼─────────┼─────────┼─────────┼───────────────────────┘
       │         │         │         │
       │         │         │         │
┌──────▼─────────▼─────────▼─────────▼───────────────────────┐
│                    HARDWARE LAYER                           │
├─────────────────────────────────────────────────────────────┤
│  4-Channel Relay Module                                     │
│  ┌───────┐  ┌───────┐  ┌───────┐  ┌───────┐               │
│  │Relay 1│  │Relay 2│  │Relay 3│  │Relay 4│               │
│  │ IN1   │  │ IN2   │  │ IN3   │  │ IN4   │               │
│  │COM NO │  │COM NO │  │COM NO │  │COM NO │               │
│  └───┬───┘  └───┬───┘  └───┬───┘  └───┬───┘               │
│      │          │          │          │                     │
│      │ 12V      │ 12V      │ 12V      │ 12V                │
│      ▼          ▼          ▼          ▼                     │
│  ┌───────┐  ┌───────┐  ┌───────┐  ┌───────┐               │
│  │Valve 1│  │Valve 2│  │Valve 3│  │Valve 4│               │
│  │  12V  │  │  12V  │  │  12V  │  │  12V  │               │
│  └───┬───┘  └───┬───┘  └───┬───┘  └───┬───┘               │
│      │          │          │          │                     │
│      ▼          ▼          ▼          ▼                     │
│  Water Flow to Garden Zones                                │
└─────────────────────────────────────────────────────────────┘
```

## Component Interactions

### Data Flow: Manual Control

```
User clicks "Start" on Zone 1
    │
    ▼
Web Browser sends POST /api/zone/1/start
    │
    ▼
ESP32 receives request
    │
    ▼
Validates zone ID and duration
    │
    ▼
Calls zones[0]->start(duration)
    │
    ▼
WateringZone sets GPIO 1 HIGH
    │
    ▼
Relay 1 activates
    │
    ▼
Valve 1 opens
    │
    ▼
Water flows
    │
    ▼
Loop() calls zones[0]->update()
    │
    ▼
Checks if duration expired
    │
    ▼
If expired: GPIO 1 LOW
    │
    ▼
Relay 1 deactivates
    │
    ▼
Valve 1 closes
    │
    ▼
Web UI updates (status polling)
```

### Data Flow: Scheduled Watering

```
NTP sync on boot
    │
    ▼
Time synchronized
    │
    ▼
Loop() calls checkSchedules()
    │
    ▼
Gets current hour & minute
    │
    ▼
For each zone:
  Check 3 schedules
    │
    ▼
If schedule matches AND enabled AND not running
    │
    ▼
Start zone with scheduled duration
    │
    ▼
(Same flow as manual control)
```

## Class Structure

### WateringZone Class

```cpp
class WateringZone {
  private:
    uint8_t pin;              // GPIO pin number
    uint8_t zoneId;           // Zone identifier (1-4)
    bool isActive;            // Current state
    unsigned long startTime;  // millis() when started
    uint32_t duration;        // Duration in milliseconds
    String name;              // Zone name
    WateringSchedule schedules[3];  // Up to 3 schedules
    
  public:
    // Control methods
    void start(uint32_t durationMs);
    void stop();
    void update();  // Called in loop()
    
    // Status methods
    bool getStatus();
    unsigned long getRemainingTime();
    
    // Configuration methods
    void setSchedule(index, enabled, hour, minute, duration);
    WateringSchedule* getSchedule(index);
    void setName(String newName);
    String getName();
    uint8_t getId();
    
    // Scheduling
    void checkSchedule(currentHour, currentMinute);
};
```

### WateringSchedule Structure

```cpp
struct WateringSchedule {
  bool enabled;        // Is this schedule active?
  uint8_t hour;        // Hour (0-23)
  uint8_t minute;      // Minute (0-59)
  uint32_t duration;   // Duration in milliseconds
};
```

## API Endpoints

### GET /
Returns the main HTML interface (embedded in firmware)

### GET /api/status
Returns JSON with complete system status:
```json
{
  "wifi": true,
  "ip": "192.168.1.100",
  "time": "14:35:22",
  "zones": [
    {
      "id": 1,
      "name": "Zone 1",
      "active": true,
      "remaining": 450,
      "schedules": [
        {
          "enabled": true,
          "hour": 6,
          "minute": 0,
          "duration": 600000
        },
        {...},
        {...}
      ]
    },
    {...}
  ]
}
```

### POST /api/zone/{id}/start
Start a zone with specified duration
```json
Request:
{
  "duration": 600000  // milliseconds
}

Response:
{
  "success": true
}
```

### POST /api/zone/{id}/stop
Stop a zone immediately
```json
Response:
{
  "success": true
}
```

### POST /api/zone/{id}/schedule
Update zone schedules
```json
Request:
{
  "schedules": [
    {
      "enabled": true,
      "hour": 6,
      "minute": 0,
      "duration": 600000
    },
    {
      "enabled": true,
      "hour": 18,
      "minute": 0,
      "duration": 300000
    },
    {
      "enabled": false,
      "hour": 0,
      "minute": 0,
      "duration": 600000
    }
  ]
}

Response:
{
  "success": true
}
```

## Storage Structure (NVS)

Data stored in ESP32 Non-Volatile Storage:

```
Namespace: "watering"

Keys:
- zone0_name: "Zone 1"          // Zone name
- zone1_name: "Zone 2"
- zone2_name: "Zone 3"
- zone3_name: "Zone 4"

- z0_s0_en: true                // Zone 0, Schedule 0, Enabled
- z0_s0_h: 6                    // Hour
- z0_s0_m: 0                    // Minute
- z0_s0_d: 600000               // Duration (ms)

- z0_s1_en: false               // Zone 0, Schedule 1
- z0_s1_h: 18
- z0_s1_m: 0
- z0_s1_d: 300000

... (repeated for all zones and schedules)
```

## Timing Diagram

```
Time:  00:00   06:00   06:10   18:00   18:05
        │       │       │       │       │
Zone 1: ────────█████████───────────────────  (Scheduled: 6:00, 10min)
Zone 2: ────────────────────────────────────  (Not scheduled)
Zone 3: ────────────────────────█████────────  (Scheduled: 18:00, 5min)
Zone 4: ────────────────────────────────────  (Not scheduled)

Legend:
─ = Inactive (Valve closed)
█ = Active (Valve open, water flowing)
```

## Memory Layout

```
ESP32-S3 N16R8:
┌────────────────────┐
│ Flash: 16MB        │
├────────────────────┤
│ - Firmware (~1MB)  │
│ - File System      │
│   (SPIFFS/LittleFS)│
│ - OTA partition    │
│ - NVS (Config)     │
│ - Free space       │
└────────────────────┘

┌────────────────────┐
│ PSRAM: 8MB         │
├────────────────────┤
│ - Dynamic memory   │
│ - Web server       │
│   buffers          │
│ - JSON documents   │
│ - Free space       │
└────────────────────┘

┌────────────────────┐
│ RAM: ~512KB        │
├────────────────────┤
│ - Stack            │
│ - Heap             │
│ - Global variables │
│ - WiFi/BT buffers  │
└────────────────────┘
```

## State Machine

### Zone State Machine

```
         ┌─────────┐
    ┌────│  IDLE   │────┐
    │    └─────────┘    │
    │                   │
stop()               start(duration)
    │                   │
    │    ┌─────────┐    │
    └────│ ACTIVE  │◄───┘
         └─────────┘
              │
              │ update()
              │ (check if duration expired)
              │
              ▼
         ┌─────────┐
         │  IDLE   │
         └─────────┘
```

### Scheduling State

```
Loop iteration
    │
    ▼
Get current time
    │
    ▼
For each zone:
    │
    ▼
For each schedule (1-3):
    │
    ▼
    ┌────────────────────┐
    │ Schedule enabled?  │───No───┐
    └────────────────────┘        │
            │ Yes                 │
            ▼                     │
    ┌────────────────────┐        │
    │ Time matches?      │───No───┤
    └────────────────────┘        │
            │ Yes                 │
            ▼                     │
    ┌────────────────────┐        │
    │ Zone not running?  │───No───┤
    └────────────────────┘        │
            │ Yes                 │
            ▼                     │
    Start zone with              │
    scheduled duration            │
            │                     │
            └─────────────────────┘
                    │
                    ▼
            Continue to next
```

## Web Interface Architecture

```
HTML (Embedded in main.cpp)
    │
    ├── CSS (Inline styles)
    │   ├── Gradient background
    │   ├── Card layouts
    │   ├── Button styles
    │   └── Responsive design
    │
    └── JavaScript
        ├── fetchStatus() - Polls every 2s
        ├── updateUI() - Updates DOM
        ├── startZone() - POST to start
        ├── stopZone() - POST to stop
        └── saveSchedules() - POST schedules
```

## Security Considerations

Currently implemented:
- Server runs on local network only
- No authentication (v1.0)
- Input validation on server side
- Duration limits enforced
- Safe defaults on error

Planned for future versions:
- Password protection
- User authentication
- HTTPS support
- API token authentication
- Rate limiting

## Performance Characteristics

- Web page load: ~2 seconds
- API response: <200ms
- Status polling interval: 2 seconds
- Schedule check interval: 1 minute
- Loop iteration: ~100ms
- WiFi reconnect: <10 seconds
- NTP sync: <5 seconds

---

**Note**: This architecture document describes version 1.0.0. See ROADMAP.md for planned architectural changes.
