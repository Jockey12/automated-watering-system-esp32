#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// Web Server Configuration
#define WEB_SERVER_PORT 80

// Hardware Pin Configuration for ESP32-S3
// Relay/Solenoid Control Pins
#define ZONE1_PIN 1
#define ZONE2_PIN 2
#define ZONE3_PIN 3
#define ZONE4_PIN 4

// Number of watering zones
#define NUM_ZONES 4

// Safety timeout (milliseconds) - maximum watering duration per zone
#define MAX_WATERING_DURATION 3600000  // 1 hour

// Default watering duration (milliseconds)
#define DEFAULT_WATERING_DURATION 600000  // 10 minutes

// NVS namespace for storing configuration
#define NVS_NAMESPACE "watering"

#endif
