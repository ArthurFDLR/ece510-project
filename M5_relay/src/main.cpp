#include <M5StickCPlus.h>
#include <WiFi.h>
// Open source MQTT Library
#include <PubSubClient.h>

// Configuration
#include "config.h"

// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

// State of the relay on or off
bool relay_state = false;

// Keep track of time to send regular availability messages
unsigned long target_time = 0L;



int counter(0);

void displayStatus();

// function called to publish the state of the relay (on/off)
void publishRelayState() {
  if (relay_state) {
    client.publish(MQTT_M5_STATE_TOPIC, MQTT_PAYLOAD_ON, true);
  } else {
    client.publish(MQTT_M5_STATE_TOPIC, MQTT_PAYLOAD_OFF, true);
  }
}

// function called to publish if the M5 is available or not
void publishRelayAvailability(bool available) {
  if (available) {
    client.publish(MQTT_M5_STATE_TOPIC, MQTT_DEVICE_AVAILABLE, true);
  } else {
    client.publish(MQTT_M5_STATE_TOPIC, MQTT_DEVICE_UNAVAILABLE, true);
  }
}


void setup_wifi() {

  delay(10);
  
  // Print to serial monitor
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // Initialize the screen and print wifi state
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(5);
  M5.Lcd.printf("Connect ");
  M5.Lcd.println(WIFI_SSID);

  // We start by connecting to a WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);

  // While not connected to the Wifi network print dots
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
  }

  randomSeed(micros());

  // Indicates that the device is connected
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(5);
  M5.Lcd.printf("Connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on the topic : ");
  Serial.print(topic);
  Serial.println();

  // concat the payload into a string
  String payload_str;
  for (uint8_t i = 0; i < length; i++) {
    payload_str.concat((char)payload[i]);
  }
  if (strcmp(topic,MQTT_M5_COMMAND_TOPIC) == 0) {
      if (payload_str.equals(String(MQTT_PAYLOAD_ON))) {
      Serial.println("Switch on the lamp");
      relay_state = true;
      digitalWrite(RELAY_PIN, HIGH);
      publishRelayState();
    } else if (payload_str.equals(String(MQTT_PAYLOAD_OFF))) {
      Serial.println("Switch off the lamp");
      relay_state = false;
      digitalWrite(RELAY_PIN, LOW);
      publishRelayState();
    } 
  }
  
  displayStatus();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {

    Serial.print("Attempting MQTT connection...");
    String clientId = "MQTT_client_M5";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_M5_COMMAND_TOPIC);
      publishRelayState();
      publishRelayAvailability(true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

// Initialize M5
M5.begin();

// Initialize the screen
M5.Lcd.setRotation(1);
M5.Lcd.fillScreen(BLACK);
M5.Lcd.setCursor(15, 10);
M5.Lcd.setTextColor(WHITE);
M5.Lcd.setTextSize(5);
M5.Lcd.printf("RELAY \nCONTROLLER");

Serial.begin(115200);

// Initialize WiFi
setup_wifi();

// Initialize the MQTT server address and port
client.setServer(MQTT_SERVER, 1883);
publishRelayAvailability(true);
client.setCallback(callback);

// Initialize the pins
pinMode(BUTTON_A_PIN, INPUT);
pinMode(RELAY_PIN, OUTPUT);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Send Availability msg at every intervals
  if (millis() - target_time >= TIME_INTERVAL_AVAILABILITY_MSG) 
  {
   target_time += TIME_INTERVAL_AVAILABILITY_MSG;
   publishRelayAvailability(true);
  }

}

void displayStatus() {
  if (relay_state) {
    // relay is on
    M5.Lcd.fillScreen(GREEN); //GREEN
    M5.Lcd.setCursor(18, 10);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(5);
    M5.Lcd.printf("ON");
  } else {
    // relay is off
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setCursor(7, 14);
    M5.Lcd.setTextColor(0xFFE0); //yellow
    M5.Lcd.setTextSize(4);
    M5.Lcd.printf("OFF");     
  }
}