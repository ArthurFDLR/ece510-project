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

bool relay_state = false;

int counter(0);

void displayStatus();

// function called to publish the state of the light (on/off)
void publishRelayState() {
  if (relay_state) {
    client.publish(MQTT_LIGHT_STATE_TOPIC, MQTT_PAYLOAD_ON, true);
  } else {
    client.publish(MQTT_LIGHT_STATE_TOPIC, MQTT_PAYLOAD_OFF, true);
  }
}


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
  // concat the payload into a string
  String payload_str;
  for (uint8_t i = 0; i < length; i++) {
    payload_str.concat((char)payload[i]);
  }
  if (strcmp(topic,"lamp") == 0) {
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
      client.subscribe(MQTT_LIGHT_COMMAND_TOPIC);
      publishRelayState();
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
M5.Lcd.printf("RELAY \nCONTROLLER");

Serial.begin(115200);

setup_wifi();
client.setServer(MQTT_SERVER, 1883);
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