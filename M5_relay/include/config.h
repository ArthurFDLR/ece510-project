// Wifi configuration
#define WIFI_SSID "TP-Link_AF58"
#define WIFI_PASSWD "74179510"

// MQTT configuration
#define MQTT_SERVER "192.168.0.151"

// State
#define MQTT_M5_STATE_TOPIC "devices/M5-relay-fan/status"
#define MQTT_M5_COMMAND_TOPIC "devices/M5-relay-fan/switch"

//Availability
#define MQTT_DEVICE_AVAILABLE "online"
#define MQTT_DEVICE_UNAVAILABLE "offline"
#define TIME_INTERVAL_AVAILABILITY_MSG 2*60*1000UL

// Commands
#define MQTT_PAYLOAD_ON "ON"
#define MQTT_PAYLOAD_OFF "OFF"

// RELAY CONTROL PIN
#define RELAY_PIN 26