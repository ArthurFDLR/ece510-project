// Wifi configuration
#define WIFI_SSID "TP-Link_AF58"
#define WIFI_PASSWD "74179510"

// MQTT configuration
#define MQTT_SERVER "192.168.0.151"

#define MQTT_LIGHT_STATE_TOPIC "devices/M5-rgb-light/status"
#define MQTT_LIGHT_COMMAND_TOPIC "devices/M5-rgb-light/switch"

#define MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC "devices/M5-rgb-light/brightness/status"
#define MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC "devices/M5-rgb-light/brightness/set"

#define MQTT_PAYLOAD_ON "ON"
#define MQTT_PAYLOAD_OFF "OFF"

#define LIGHT_PIN 9