#ifndef WATERING_ZONE_H
#define WATERING_ZONE_H

#include <Arduino.h>

struct WateringSchedule {
  bool enabled;
  uint8_t hour;
  uint8_t minute;
  uint32_t duration;  // Duration in milliseconds
};

class WateringZone {
  private:
    uint8_t pin;
    uint8_t zoneId;
    bool isActive;
    unsigned long startTime;
    uint32_t duration;
    String name;
    WateringSchedule schedules[3];  // Up to 3 schedules per zone
    
  public:
    WateringZone(uint8_t zonePin, uint8_t id, String zoneName);
    void begin();
    void start(uint32_t durationMs);
    void stop();
    void update();
    bool getStatus();
    unsigned long getRemainingTime();
    void setSchedule(uint8_t index, bool enabled, uint8_t hour, uint8_t minute, uint32_t duration);
    WateringSchedule* getSchedule(uint8_t index);
    String getName();
    void setName(String newName);
    uint8_t getId();
    void checkSchedule(uint8_t currentHour, uint8_t currentMinute);
};

#endif
