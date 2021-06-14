#include "M5StickCPlus.h"
#include <WiFi.h>

char* ssid       = "FakeWifiHotspot";
char* password   = "password";

void setup() {
    M5.begin(true, true, true);
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
}

void loop() {
}