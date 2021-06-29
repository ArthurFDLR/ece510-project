// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

void setup() {
  Serial.begin(9600);// start serial for output
  pinMode(7, OUTPUT);
  pinMode(3, INPUT);
}

void loop() {
  digitalWrite(7, digitalRead(3));
  Serial.println(digitalRead(3));
}
