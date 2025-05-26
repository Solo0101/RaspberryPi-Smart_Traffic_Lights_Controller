#ifndef TRAFFIC_LIGHT_CONTROLLER_H
#define TRAFFIC_LIGHT_CONTROLLER_H

#include <Arduino.h>
#include <HCSR04.h>

class TrafficLightController {
public:
  TrafficLightController(int nr, int ny, int ng, int sr, int sy, int sg, int er, int ey, int eg, int wr, int wy, int wg);

  void update();
  void reportStatus();
  void handleSerialCommand(String command);

  enum State {
    NORTH_SOUTH_GREEN,
    NORTH_SOUTH_YELLOW,
    EAST_WEST_GREEN,
    EAST_WEST_YELLOW,
    ALL_RED
  };

private:
  // Pin configuration
  int northRed, northYellow, northGreen;
  int southRed, southYellow, southGreen;
  int eastRed, eastYellow, eastGreen;
  int westRed, westYellow, westGreen;

  // FSM state
  State currentState;
  unsigned long lastStateChange = 0;
  unsigned long lastReportTime = 0;
  unsigned long allRedStartTime = 0;
  
  const unsigned long reporTimeInterval = 1000;
  const unsigned long milisecondsInSecond = 1000;
  const unsigned long allRedTimeout = 10000;

  // Phase durations (ms)
  unsigned long northSouthGreenTime = 42000;
  unsigned long northSouthYellowTime = 3000;
  unsigned long eastWestGreenTime = 42000;
  unsigned long eastWestYellowTime = 3000;


  // Pending duration adjustments
  bool nsIncrease = false;
  bool nsDecrease = false;
  bool ewIncrease = false;
  bool ewDecrease = false;
  bool nsAdjusted = false;
  bool ewAdjusted = false;


  // Tramway management
  bool emergencyActive = false;
  bool sensorHoldActive = false;

  UltraSonicDistanceSensor* distanceSensorNorth = new UltraSonicDistanceSensor(15, 14);
  UltraSonicDistanceSensor* distanceSensorSouth = new UltraSonicDistanceSensor(17, 16);
  const unsigned int distanceThreshold = 5;

  bool checkNorthSensor();
  bool checkSouthSensor();

  void transitionTo(State nextState);

  // LED control helpers
  void setNorthSouthGreen();
  void setNorthSouthYellow();
  void setEastWestGreen();
  void setEastWestYellow();
  void setAllRed();
};

#endif
