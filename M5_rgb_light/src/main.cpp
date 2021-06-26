#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>

/** JSON RGB light state information
  {
   "brightness": 255,
   "color_mode": "rgb",
   "color_temp": 155,
   "color": {
     "r": 255,
     "g": 180,
     "b": 200,
     "c": 100,
     "w": 50,
     "x": 0.406,
     "y": 0.301,
     "h": 344.0,
     "s": 29.412
  },
   "effect": "colorloop",
   "state": "ON",
   "transition": 2,
   "white_value": 150
  }

*/

// SYSTEM CONTROL PIN
#define LAMP_PIN 26

// WIFI & MQTT configuration
#include "config.h"

//MQTT client
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

bool LampState = false;

int counter(0);

void displayStatus();

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
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
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on the topic : ");
  Serial.print(topic);
  Serial.println();
  if (strcmp(topic,"lamp") == 0) {
      if ((char)payload[0] == '1') {
      Serial.println("Switch on the lamp");
      LampState = true;
      digitalWrite(LAMP_PIN, HIGH);
    } else if ((char)payload[0] == '2') {
      Serial.println("Switch the lamp");
      if (LampState) {
        digitalWrite(LAMP_PIN, LOW);
      } else {
        digitalWrite(LAMP_PIN, HIGH);
      }
      LampState = !LampState;
    } else {
      Serial.println("Switch off the lamp");  
      LampState = false;
      digitalWrite(LAMP_PIN, LOW);
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
      client.subscribe("lamp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
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
M5.Lcd.setTextSize(5);
M5.Lcd.printf("LAMP \nCONTROLLER");

Serial.begin(115200);

setup_wifi();
client.setServer(MQTT_SERVER, 1883);
client.setCallback(callback);

// Innitialize the pins
pinMode(BUTTON_A_PIN, INPUT);
pinMode(LAMP_PIN, OUTPUT);
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
  if (LampState) {
    // Lamp is on
    M5.Lcd.fillScreen(GREEN); //GREEN
    M5.Lcd.setCursor(18, 10);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(5);
    M5.Lcd.printf("ON");
  } else {
    // Lamp is off
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setCursor(7, 14);
    M5.Lcd.setTextColor(0xFFE0); //yellow
    M5.Lcd.setTextSize(4);
    M5.Lcd.printf("OFF");     
  }
}