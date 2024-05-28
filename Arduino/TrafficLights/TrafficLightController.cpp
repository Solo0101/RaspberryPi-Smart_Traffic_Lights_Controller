#include "TrafficLightController.h"

TrafficLightController::TrafficLightController(int nr, int ny, int ng, int sr, int sy, int sg, int er, int ey, int eg, int wr, int wy, int wg) {
  northRed = nr;
  northYellow = ny;
  northGreen = ng;
  southRed = sr;
  southYellow = sy;
  southGreen = sg;
  eastRed = er;
  eastYellow = ey;
  eastGreen = eg;
  westRed = wr;
  westYellow = wy;
  westGreen = wg;

  // Initialize all LED pins as outputs
  pinMode(northRed, OUTPUT);
  pinMode(northYellow, OUTPUT);
  pinMode(northGreen, OUTPUT);
  pinMode(southRed, OUTPUT);
  pinMode(southYellow, OUTPUT);
  pinMode(southGreen, OUTPUT);
  pinMode(eastRed, OUTPUT);
  pinMode(eastYellow, OUTPUT);
  pinMode(eastGreen, OUTPUT);
  pinMode(westRed, OUTPUT);
  pinMode(westYellow, OUTPUT);
  pinMode(westGreen, OUTPUT);

  // Start with north and south direction green, others red
  //setNorthSouthGreen();
  //currentState = NORTH_SOUTH_GREEN;
  setAllRed();
  currentState = ALL_RED;
  //Serial.println("In constructor");
}

void TrafficLightController::defaultCycleWithDistanceSensor() {
  
  if (checkNorthSensor() || checkSouthSensor()) {
    setNorthSouthGreen();
    currentState = NORTH_SOUTH_GREEN;
    delay(greenTime);
  } else {
    // Cycle through each direction normally
    switch (currentState) {
      case NORTH_SOUTH_GREEN:
        setNorthSouthGreen();
        delay(greenTime);
        setNorthSouthYellow();
        delay(yellowTime);
        setAllRed();
        delay(redTime);
        currentState = EAST_WEST_GREEN;
        break;
      case EAST_WEST_GREEN:
        setEastWestGreen();
        delay(greenTime);
        setEastWestYellow();
        delay(yellowTime);
        setAllRed();
        delay(redTime);
        currentState = NORTH_SOUTH_GREEN;
        break;
      case ALL_RED: 
        setAllRed();
        delay(redTime);
        currentState = NORTH_SOUTH_GREEN;
        break;
      default:
        currentState = ALL_RED;
        setAllRed();
        delay(redTime);
        currentState = NORTH_SOUTH_GREEN;
        setNorthSouthGreen();
        delay(greenTime);
        break;
    }
    
  }
}

bool TrafficLightController::checkNorthSensor() {
      
      return (distanceSensorNorth->measureDistanceCm() < distanceThreshold && distanceSensorNorth->measureDistanceCm() > 0);
    }

bool TrafficLightController::checkSouthSensor() {
      return (distanceSensorSouth->measureDistanceCm() < distanceThreshold && distanceSensorNorth->measureDistanceCm() > 0);
    }

void TrafficLightController::handleSerialCommand(String command) {
  if (command.equals("NorthSouth")) {
    setNorthSouthGreen();
    delay(greenTime);
    currentState = NORTH_SOUTH_GREEN;
  } else if (command.equals("EastWest")) {
    setEastWestGreen();
    delay(greenTime);
    currentState = EAST_WEST_GREEN;
  } else if (command.equals("AllRed")) {
    setAllRed();
    delay(redTime);
    currentState = ALL_RED;
  }
}

void TrafficLightController::setNorthSouthGreen() {
      // Set north and south green, east and west red
      digitalWrite(northGreen, HIGH);
      digitalWrite(northYellow, LOW);
      digitalWrite(northRed, LOW);

      digitalWrite(southGreen, HIGH);
      digitalWrite(southYellow, LOW);
      digitalWrite(southRed, LOW);

      digitalWrite(eastGreen, LOW);
      digitalWrite(eastYellow, LOW);
      digitalWrite(eastRed, HIGH);

      digitalWrite(westGreen, LOW);
      digitalWrite(westYellow, LOW);
      digitalWrite(westRed, HIGH);
    }

void TrafficLightController::setNorthSouthYellow() {
      // Set north and south yellow, east and west red
      digitalWrite(northGreen, LOW);
      digitalWrite(northYellow, HIGH);
      digitalWrite(northRed, LOW);

      digitalWrite(southGreen, LOW);
      digitalWrite(southYellow, HIGH);
      digitalWrite(southRed, LOW);

      digitalWrite(eastGreen, LOW);
      digitalWrite(eastYellow, LOW);
      digitalWrite(eastRed, HIGH);

      digitalWrite(westGreen, LOW);
      digitalWrite(westYellow, LOW);
      digitalWrite(westRed, HIGH);
    }

void TrafficLightController::setEastWestGreen() {
      // Set east and west green, north and south red
      digitalWrite(northGreen, LOW);
      digitalWrite(northYellow, LOW);
      digitalWrite(northRed, HIGH);

      digitalWrite(southGreen, LOW);
      digitalWrite(southYellow, LOW);
      digitalWrite(southRed, HIGH);

      digitalWrite(eastGreen, HIGH);
      digitalWrite(eastYellow, LOW);
      digitalWrite(eastRed, LOW);

      digitalWrite(westGreen, HIGH);
      digitalWrite(westYellow, LOW);
      digitalWrite(westRed, LOW);
    }

void TrafficLightController::setEastWestYellow() {
      // Set east and west yellow, north and south red
      digitalWrite(northGreen, LOW);
      digitalWrite(northYellow, LOW);
      digitalWrite(northRed, HIGH);

      digitalWrite(southGreen, LOW);
      digitalWrite(southYellow, LOW);
      digitalWrite(southRed, HIGH);

      digitalWrite(eastGreen, LOW);
      digitalWrite(eastYellow, HIGH);
      digitalWrite(eastRed, LOW);

      digitalWrite(westGreen, LOW);
      digitalWrite(westYellow, HIGH);
      digitalWrite(westRed, LOW);
    }

void TrafficLightController::setAllRed() {
      // Set all directions red
      digitalWrite(northGreen, LOW);
      digitalWrite(northYellow, LOW);
      digitalWrite(northRed, HIGH);

      digitalWrite(southGreen, LOW);
      digitalWrite(southYellow, LOW);
      digitalWrite(southRed, HIGH);

      digitalWrite(eastGreen, LOW);
      digitalWrite(eastYellow, LOW);
      digitalWrite(eastRed, HIGH);

      digitalWrite(westGreen, LOW);
      digitalWrite(westYellow, LOW);
      digitalWrite(westRed, HIGH);
    }
