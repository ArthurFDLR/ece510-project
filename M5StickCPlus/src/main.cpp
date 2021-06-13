#include "M5StickCPlus.h"
#include "WiFi.h"


char WIFI_SSID[] =  "TP-Link_AF58";
const char* WIFI_PASS = "74179510";

void setup() {
  M5.begin();
  Serial.begin(115200);
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("bonjour");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("CONNECTED");
}

void loop() {
    Serial.println("Loop");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected");
        Serial.println(WiFi.localIP().toString());
    } else {
        Serial.println("WiFi not connected");
    }
    

}