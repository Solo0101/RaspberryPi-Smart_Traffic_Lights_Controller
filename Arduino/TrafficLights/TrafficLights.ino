#include "TrafficLightController.h"
#include <Arduino.h>

TrafficLightController traffic_light_controller(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);

void setup() {
  Serial.begin(9600);
}

void loop() {
  if(traffic_light_controller.currentState == TrafficLightController::State::NORTH_SOUTH_GREEN) {
    Serial.println("NorthSouth");
  } else if(traffic_light_controller.currentState == TrafficLightController::State::EAST_WEST_GREEN) {
    Serial.println("EastWest");
  } else if(traffic_light_controller.currentState == TrafficLightController::State::ALL_RED) {
    Serial.println("AllRed");
  }

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    traffic_light_controller.handleSerialCommand(command);
  } else {
    traffic_light_controller.defaultCycleWithDistanceSensor();
  }
  delay(1000);
}
