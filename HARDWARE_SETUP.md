# Hardware Setup Guide

## Components List

### Required Components

1. **ESP32-S3-DevKitC-1 (N16R8)**
   - 16MB Flash Memory
   - 8MB PSRAM
   - USB-C connector for programming

2. **4-Channel 5V Relay Module**
   - Optocoupler isolated (recommended)
   - Active LOW or HIGH (adjust code if needed)
   - Current rating: 10A @ 250VAC or 10A @ 30VDC

3. **Solenoid Valves (Quantity: 4)**
   - 12V DC (or your preferred voltage)
   - Normally Closed (NC) type recommended
   - 1/2" or 3/4" thread size (match your irrigation system)

4. **Power Supplies**
   - 5V 2A for ESP32 and relay module
   - 12V power supply (current depends on number of valves running simultaneously)
     - Each valve typically draws 0.3-0.5A
     - Recommended: 12V 2A for 4 valves

5. **Additional Components**
   - Jumper wires
   - Breadboard (for prototyping) or PCB
   - Waterproof project box
   - Cable glands for outdoor installation
   - Fuse holder and fuses (recommended)

### Optional Components

- Soil moisture sensors (for future integration)
- Water pressure sensor
- LCD display for standalone operation
- Power switch
- Status LEDs

## Detailed Wiring

### ESP32-S3 to Relay Module

```
ESP32-S3          Relay Module
--------          ------------
GPIO 1    ----->  IN1
GPIO 2    ----->  IN2
GPIO 3    ----->  IN3
GPIO 4    ----->  IN4
GND       ----->  GND
5V        ----->  VCC
```

### Relay Module to Solenoid Valves

```
Relay Module      Solenoid Valves
------------      ---------------
Relay 1 COM  ---> 12V+ (Power Supply)
Relay 1 NO   ---> Valve 1 Positive (+)
Relay 2 COM  ---> 12V+ (Power Supply)
Relay 2 NO   ---> Valve 2 Positive (+)
Relay 3 COM  ---> 12V+ (Power Supply)
Relay 3 NO   ---> Valve 3 Positive (+)
Relay 4 COM  ---> 12V+ (Power Supply)
Relay 4 NO   ---> Valve 4 Positive (+)

All Valve Negative (-) ---> 12V GND (Power Supply)
```

### Power Supply Connections

```
5V Power Supply          12V Power Supply
---------------          ----------------
5V+  ---> ESP32 5V       12V+ ---> All Relay COM terminals
GND  ---> ESP32 GND      GND  ---> All Valve (-)
      |-> Relay GND           |-> ESP32 GND (common ground)
```

**Important**: Ensure common ground between all power supplies!

## Pin Selection Notes

### ESP32-S3 GPIO Considerations

The default pins (GPIO 1-4) should work, but here are alternatives if needed:

**Safe GPIO pins for ESP32-S3:**
- GPIO 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
- GPIO 21, 33, 34, 35, 36, 37, 38

**Avoid these pins:**
- GPIO 0: Strapping pin (boot mode)
- GPIO 19, 20: USB (if using USB)
- GPIO 26-32: Used for PSRAM/Flash on N16R8 variant
- GPIO 39-48: Strapping/special function pins

### Changing Pins

To change pins, edit `src/config.h`:

```cpp
#define ZONE1_PIN 5   // Change to your preferred pin
#define ZONE2_PIN 6
#define ZONE3_PIN 7
#define ZONE4_PIN 8
```

## Assembly Steps

### 1. Prototype Setup (Breadboard)

1. Connect ESP32-S3 to breadboard
2. Connect relay module to breadboard
3. Wire ESP32 to relay module as shown above
4. Connect 5V power to ESP32 and relay module
5. DO NOT connect valves yet - test with LEDs first

### 2. Testing

1. Upload the code
2. Connect LEDs instead of relays initially
3. Use web interface to test each zone
4. Verify each pin activates correctly

### 3. Valve Connection

1. Power off all systems
2. Connect solenoid valves to relay outputs
3. Connect 12V power supply
4. Verify wiring multiple times
5. Test one valve at a time

### 4. Enclosure

1. Choose waterproof enclosure
2. Add cable glands for:
   - Power input
   - Valve control wires
   - Optional: sensor wires
3. Mount ESP32 and relay module securely
4. Ensure adequate ventilation if needed
5. Add labels for each zone

## Safety Checklist

- [ ] All connections are secure and insulated
- [ ] Common ground between all power supplies
- [ ] Fuses installed for protection
- [ ] Relay ratings match or exceed valve current
- [ ] Waterproof enclosure for outdoor use
- [ ] Cable glands properly sealed
- [ ] No exposed wiring
- [ ] Tested with multimeter before power on
- [ ] Valves tested individually before full operation
- [ ] Emergency shutoff accessible

## Wiring Diagram

```
                    ┌─────────────────┐
                    │  5V Power Supply │
                    └────────┬────────┘
                             │
                    ┌────────┴────────┐
                    │                 │
           ┌────────▼───────┐  ┌─────▼──────┐
           │   ESP32-S3     │  │   Relay    │
           │                │  │   Module   │
           │  GPIO1 ────────┼──┤ IN1   COM1 ├───┐
           │  GPIO2 ────────┼──┤ IN2   COM2 ├───┤
           │  GPIO3 ────────┼──┤ IN3   COM3 ├───┤
           │  GPIO4 ────────┼──┤ IN4   COM4 ├───┤
           │                │  │             │   │
           │  GND   ────────┼──┤ GND    NO1  ├───┼───┐
           │  5V    ────────┼──┤ VCC    NO2  ├───┼───┼───┐
           └────────────────┘  │        NO3  ├───┼───┼───┼───┐
                               │        NO4  ├───┼───┼───┼───┼───┐
                               └─────────────┘   │   │   │   │   │
                                                 │   │   │   │   │
                    ┌─────────────────┐          │   │   │   │   │
                    │ 12V Power Supply│          │   │   │   │   │
                    └────────┬────────┘          │   │   │   │   │
                             │                   │   │   │   │   │
                    ┌────────┴────────┐          │   │   │   │   │
                    │        │        │          │   │   │   │   │
              12V+ ─┘        └─ GND   │          │   │   │   │   │
                │                     │          │   │   │   │   │
                │            ┌────────┴──────────┘   │   │   │   │
                │            │    ┌───────────────────┘   │   │   │
                │            │    │    ┌──────────────────┘   │   │
                │            │    │    │    ┌─────────────────┘   │
                │            │    │    │    │                     │
             ┌──▼──┐      ┌──▼──┐ │ ┌──▼──┐ │ ┌──▼──┐           │
             │Valve│      │Valve│ │ │Valve│ │ │Valve│           │
             │  1  │      │  2  │ │ │  3  │ │ │  4  │           │
             └──┬──┘      └──┬──┘ │ └──┬──┘ │ └──┬──┘           │
                │            │    │    │    │    │               │
                └────────────┴────┴────┴────┴────┴───────────────┘
                                  GND
```

## Testing Procedure

### 1. Power Test
```bash
1. Connect only 5V power
2. ESP32 should boot and connect to WiFi
3. Serial monitor shows IP address
4. Web interface accessible
```

### 2. Relay Test
```bash
1. Click "Start" for Zone 1 with 1-minute duration
2. Listen for relay click
3. Verify relay LED lights up
4. Wait for automatic shutoff
5. Repeat for all zones
```

### 3. Valve Test
```bash
1. Ensure water supply is on
2. Start with shortest duration (1 minute)
3. Start Zone 1
4. Verify water flow
5. Verify automatic shutoff
6. Check for leaks
7. Repeat for all zones
```

### 4. Schedule Test
```bash
1. Set schedule for 2 minutes in future
2. Wait and verify automatic activation
3. Verify automatic shutoff after duration
```

## Troubleshooting Hardware

### Relay Not Clicking
- Check 5V power to relay module
- Verify GPIO pin connection
- Check if pin is HIGH when activated (measure with multimeter)
- Try different GPIO pin

### Valve Not Opening
- Verify 12V power supply
- Check valve polarity (try reversing if no polarity marking)
- Test valve directly with 12V (bypass relay)
- Check for debris in valve

### ESP32 Resets When Relay Activates
- Power supply insufficient current
- Add decoupling capacitors
- Separate power supplies for ESP32 and relays
- Ensure common ground

### Water Leak
- Check all valve connections
- Verify proper thread sealant/tape
- Ensure valve is correct type (NC vs NO)
- Check maximum water pressure rating

## Maintenance

### Regular Checks
- Weekly: Check for leaks
- Monthly: Test all zones manually
- Seasonally: Clean valve filters
- Annually: Replace valve seals if needed

### Winterization
- Drain all water from valves
- Store indoor or in heated area
- Disconnect power
- Cover outdoor enclosure
