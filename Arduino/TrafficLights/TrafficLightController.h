#ifndef TRAFFIC_LIGHT_CONTROLLER_H
#define TRAFFIC_LIGHT_CONTROLLER_H

#include <Arduino.h>
#include <HCSR04.h>
#include <math.h>

class TrafficLightController {

  public:
    // Constructor to initialize the pin values
    TrafficLightController(int nr, int ny, int ng, int sr, int sy, int sg, int er, int ey, int eg, int wr, int wy, int wg, int nTrig, int nEcho, int sTrig, int sEcho);

    void defaultCycleWithDistanceSensor();

    // Enumeration for states
    enum State {
      NORTH_SOUTH_GREEN,
      NORTH_SOUTH_YELLOW,
      EAST_WEST_GREEN,
      EAST_WEST_YELLOW,
      ALL_RED
    };

    // Current state variable
    State currentState, nextState;

    UltraSonicDistanceSensor *distanceSensorNorth = new UltraSonicDistanceSensor(15, 14);
    UltraSonicDistanceSensor *distanceSensorSouth = new UltraSonicDistanceSensor(17, 16);

    bool checkNorthSensor();

    bool checkSouthSensor();

    // Handle Serial Commands, Only accepted commands are:
    // [1, 1, 0, 1, 0] -> North/South green, East/West red
    // [1, 0, 1, 0, 1] -> East/West green, North/South red
    void handleSerialCommand(String command);

  private:
    // Pin definitions for traffic lights
    int northRed, northYellow, northGreen;
    int southRed, southYellow, southGreen;
    int eastRed, eastYellow, eastGreen;
    int westRed, westYellow, westGreen;

    // Pin definitions for distance sensors
    int northTrig, northEcho;
    int southTrig, southEcho;

    // Timing constants
    const int redTime = 3000;      // Time for red light (3 seconds)
    const int yellowTime = 1000;   // Time for yellow light (1 seconds)
    const int greenTime = 3000;    // Time for green light (3 seconds)

    // Distance threshold
    const int distanceThreshold = 5; // Threshold distance in cm

    void setNorthSouthGreen();

    void setNorthSouthYellow();

    void setEastWestGreen();

    void setEastWestYellow();

    void setAllRed();
};


#endif