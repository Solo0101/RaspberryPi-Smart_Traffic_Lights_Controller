#include "TrafficLightController.h"
#include <Arduino.h>

TrafficLightController traffic_light_controller(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2, A3);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // while (!Serial) {
  //   ; // Wait for serial port to connect
  // }
}

void loop() {
  // put your main code here, to run repeatedly:
  // UltraSonicDistanceSensor distanceSensorNorth(A1, A0);
  // UltraSonicDistanceSensor distanceSensorSouth(A3, A2);
  // if (Serial.available() > 0) {
  //   // String input = Serial.readStringUntil('\n');  // Read the input from Raspberry Pi
  //   // Serial.println("Arduino received: " + input);  // Respond back with received message
    
  //   // Send a message back to Raspberry Pi
  //   // String response = "Hello from Arduino!";
  //   // Serial.println(response);
    
  // }
  // delay(1000);  // Add a small delay to avoid overwhelming the serial buffer
  // traffic_light_controller.defaultCycleWithDistanceSensor();
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    traffic_light_controller.handleSerialCommand(command);
  } else {
    traffic_light_controller.defaultCycleWithDistanceSensor();
  }
  Serial.println(traffic_light_controller.currentState);

}
