#include "WateringZone.h"
#include "config.h"

WateringZone::WateringZone(uint8_t zonePin, uint8_t id, String zoneName) {
  pin = zonePin;
  zoneId = id;
  name = zoneName;
  isActive = false;
  startTime = 0;
  duration = 0;
  
  // Initialize schedules as disabled
  for (int i = 0; i < 3; i++) {
    schedules[i].enabled = false;
    schedules[i].hour = 0;
    schedules[i].minute = 0;
    schedules[i].duration = DEFAULT_WATERING_DURATION;
  }
}

void WateringZone::begin() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);  // Relay OFF (assuming active HIGH relays)
}

void WateringZone::start(uint32_t durationMs) {
  if (durationMs > MAX_WATERING_DURATION) {
    durationMs = MAX_WATERING_DURATION;
  }
  
  isActive = true;
  startTime = millis();
  duration = durationMs;
  digitalWrite(pin, HIGH);  // Turn relay ON
  Serial.printf("Zone %d (%s) started for %lu ms\n", zoneId, name.c_str(), durationMs);
}

void WateringZone::stop() {
  isActive = false;
  digitalWrite(pin, LOW);  // Turn relay OFF
  Serial.printf("Zone %d (%s) stopped\n", zoneId, name.c_str());
}

void WateringZone::update() {
  if (isActive) {
    unsigned long elapsed = millis() - startTime;
    if (elapsed >= duration) {
      stop();
    }
  }
}

bool WateringZone::getStatus() {
  return isActive;
}

unsigned long WateringZone::getRemainingTime() {
  if (!isActive) {
    return 0;
  }
  unsigned long elapsed = millis() - startTime;
  if (elapsed >= duration) {
    return 0;
  }
  return (duration - elapsed) / 1000;  // Return in seconds
}

void WateringZone::setSchedule(uint8_t index, bool enabled, uint8_t hour, uint8_t minute, uint32_t dur) {
  if (index < 3) {
    schedules[index].enabled = enabled;
    schedules[index].hour = hour;
    schedules[index].minute = minute;
    schedules[index].duration = dur;
  }
}

WateringSchedule* WateringZone::getSchedule(uint8_t index) {
  if (index < 3) {
    return &schedules[index];
  }
  return nullptr;
}

String WateringZone::getName() {
  return name;
}

void WateringZone::setName(String newName) {
  name = newName;
}

uint8_t WateringZone::getId() {
  return zoneId;
}

void WateringZone::checkSchedule(uint8_t currentHour, uint8_t currentMinute) {
  // Check if any schedule matches current time and should trigger
  for (int i = 0; i < 3; i++) {
    if (schedules[i].enabled && 
        schedules[i].hour == currentHour && 
        schedules[i].minute == currentMinute &&
        !isActive) {  // Only start if not already running
      start(schedules[i].duration);
      break;  // Only trigger one schedule at a time
    }
  }
}
