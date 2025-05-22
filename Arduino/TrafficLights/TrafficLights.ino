#include "TrafficLightController.h"
#include <Arduino.h>

TrafficLightController trafficController(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);

void setup() {
  Serial.begin(9600);
  Serial.println("Serial connection ready");
}

String inputBuffer = "";

void loop() {
  trafficController.update();      // FSM and state handling
  trafficController.reportStatus(); // Report state every second

  // Non-blocking read from Serial
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        Serial.println("Received command " + inputBuffer);
        trafficController.handleSerialCommand(inputBuffer);
        inputBuffer = "";
      }
    } else {
      inputBuffer += c;
    }
  }
}
