#include "TrafficLightController.h"

TrafficLightController::TrafficLightController(int nr, int ny, int ng, int sr, int sy, int sg, int er, int ey, int eg, int wr, int wy, int wg, int nTrig, int nEcho, int sTrig, int sEcho) {
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

  northTrig = nTrig;
  northEcho = nEcho;
  southTrig = sTrig;
  southEcho = sEcho;

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

  // Initialize sensor pins
  // pinMode(northTrig, OUTPUT);
  // pinMode(northEcho, INPUT);
  // pinMode(southTrig, OUTPUT);
  // pinMode(southEcho, INPUT);
  // distanceSensorNorth = new UltraSonicDistanceSensor(northTrig, northEcho);
  // distanceSensorSouth = new UltraSonicDistanceSensor(southTrig, southEcho);

  // Start with north and south direction green, others red
  setNorthSouthGreen();
  currentState = NORTH_SOUTH_GREEN;
  //Serial.println("In constructor");
}

void TrafficLightController::defaultCycleWithDistanceSensor() {
  //Serial.println(currentState);
  // Check sensors and prioritize north/south if needed
  if (checkNorthSensor() || checkSouthSensor()) {
    setNorthSouthGreen();
    currentState = NORTH_SOUTH_GREEN;
    delay(greenTime);
  } else {
    // Cycle through each direction normally
    switch (currentState) {
      case NORTH_SOUTH_GREEN:
        setNorthSouthYellow();
        delay(yellowTime);
        setAllRed();
        delay(redTime);
        setEastWestGreen();
        currentState = EAST_WEST_GREEN;
        delay(greenTime);
        break;
      case EAST_WEST_GREEN:
        setEastWestYellow();
        delay(yellowTime);
        setAllRed();
        delay(redTime);
        setNorthSouthGreen();
        currentState = NORTH_SOUTH_GREEN;
        delay(greenTime);
        break;
      case ALL_RED: 
        setAllRed();
        delay(redTime);
        currentState = NORTH_SOUTH_GREEN;
        break;
      default:
        setAllRed();
        currentState = ALL_RED;
        delay(redTime);
        setNorthSouthGreen();
        currentState = NORTH_SOUTH_GREEN;
        delay(greenTime);
        break;
    }
    
  }

  
  // switch (currentState) {
  //   //Serial.println((int)currentState);
  //     case NORTH_SOUTH_GREEN:
  //       setNorthSouthYellow();
  //       //currentState = NORTH_SOUTH_YELLOW;
  //       delay(yellowTime);
  //       setAllRed();
  //       //currentState = ALL_RED;
  //       delay(redTime);
  //       setEastWestGreen();
  //       currentState = EAST_WEST_GREEN;
  //       delay(greenTime);
  //       break;
  //     case EAST_WEST_GREEN:
  //       setEastWestYellow();
  //       //currentState = EAST_WEST_YELLOW;
  //       delay(yellowTime);
  //       setAllRed();
  //       //currentState = ALL_RED;
  //       delay(redTime);
  //       setNorthSouthGreen();
  //       currentState = NORTH_SOUTH_GREEN;
  //       delay(greenTime);
  //       break;
  //     default:
  //       setAllRed();
  //       //currentState = ALL_RED;
  //       delay(redTime);
  //       setNorthSouthGreen();
  //       currentState = NORTH_SOUTH_GREEN;
  //       delay(greenTime);
  //       break;
  //   }
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
