#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>

// configuration
#include "config.h"

// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

boolean light_state       = false;
uint8_t light_brightness  = 255;

int counter(0);

void displayStatus();

// function called to publish the state of the light (on/off)
void publishLightState() {
  if (light_state) {
    client.publish(MQTT_LIGHT_STATE_TOPIC, MQTT_PAYLOAD_ON, true);
  } else {
    client.publish(MQTT_LIGHT_STATE_TOPIC, MQTT_PAYLOAD_OFF, true);
  }
}

// function called to publish the brightness of the led
void publishLightBrightness() {
  snprintf(msg, MSG_BUFFER_SIZE, "%d", light_brightness);
  client.publish(MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC, msg, true);
}

// function called to adapt the brightness and the state of the led
void setLightState() {
  if (light_state) {
    Wire.beginTransmission(42);
    Wire.write(light_brightness);
    Wire.endTransmission();
    Serial.print("INFO: Brightness: ");
    Serial.println(light_brightness);
  } else {
    Wire.beginTransmission(42);
    Wire.write(0);
    Wire.endTransmission();
    Serial.println("INFO: Turn light off");
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(5);
  M5.Lcd.printf("Connect ");
  M5.Lcd.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
  }

  randomSeed(micros());

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
  if (strcmp(topic,MQTT_LIGHT_COMMAND_TOPIC) == 0) {
      if (payload_str.equals(String(MQTT_PAYLOAD_ON))) {
      Serial.println("Switch on the lamp");
      light_state = true;
      Wire.beginTransmission(42);
      Wire.write(255);
      Wire.endTransmission();
      publishLightState();
    } else if (payload_str.equals(String(MQTT_PAYLOAD_OFF))) {
      Serial.println("Switch off the lamp");
      light_state = false;
      Wire.beginTransmission(42);
      Wire.write(0);
      Wire.endTransmission();
      publishLightState();
    } 
  } else if (String(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC).equals(topic)) {
    uint8_t brightness = payload_str.toInt();
    if (brightness < 0 || brightness > 255) {
      // do nothing
      return;
    } else {
      light_brightness = brightness;
      setLightState();
      publishLightBrightness();
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
      client.subscribe(MQTT_LIGHT_COMMAND_TOPIC);
      publishLightState();
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

M5.begin();

// Initialize the screen
M5.Lcd.setRotation(1);
M5.Lcd.fillScreen(BLACK);
M5.Lcd.setCursor(15, 10);
M5.Lcd.setTextColor(WHITE);
M5.Lcd.setTextSize(3);
M5.Lcd.printf("LIGHT \nCONTROLLER");

Serial.begin(115200);

setup_wifi();
client.setServer(MQTT_SERVER, 1883);
client.setCallback(callback);

// Initialize I2C connection
Wire.begin(0,26);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // test the publish function
  if(digitalRead(BUTTON_A_PIN) == 0) {
    char counter_str[5];
    client.publish("esp32", itoa(counter,counter_str,10));
    Serial.print("IP: ");
    Serial.print(MQTT_SERVER);
    Serial.print("\tTopic: ");
    Serial.print("esp32\t Message: ");
    Serial.println(counter);
    counter += 1;
    delay(500); 
  };
}

void displayStatus() {
  if (light_state) {
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