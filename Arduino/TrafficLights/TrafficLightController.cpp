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

  currentState = NORTH_SOUTH_GREEN;
  transitionTo(currentState);
}

void TrafficLightController::update() {
  unsigned long now = millis();

  // Trigger a priority request if sensors detect vehicles
  if (!inPriorityTransition && (checkNorthSensor() || checkSouthSensor())) {
    if (currentState != NORTH_SOUTH_GREEN && currentState != NORTH_SOUTH_YELLOW && currentState != ALL_RED) {
      priorityRequested = true;
    }
  }

  switch (currentState) {
    case NORTH_SOUTH_GREEN:
      if (now - lastStateChange >= northSouthGreenTime) {
        if (nsIncrease) {
          northSouthGreenTime = min(northSouthGreenTime + 1000, 60000);
          nsIncrease = false;
        }
        if (nsDecrease) {
          northSouthGreenTime = max(northSouthGreenTime - 1000, 10000);
          nsDecrease = false;
        }
        transitionTo(NORTH_SOUTH_YELLOW);
      }
      break;

    case NORTH_SOUTH_YELLOW:
      if (now - lastStateChange >= northSouthYellowTime) {
        if (priorityRequested) {
          inPriorityTransition = true;
          priorityRequested = false;
          transitionTo(ALL_RED);
        } else {
          transitionTo(EAST_WEST_GREEN);
        }
      }
      break;

    case EAST_WEST_GREEN:
      if (now - lastStateChange >= eastWestGreenTime) {
        if (ewIncrease) {
          eastWestGreenTime = min(eastWestGreenTime + 1000, 60000);
          ewIncrease = false;
        }
        if (ewDecrease) {
          eastWestGreenTime = max(eastWestGreenTime - 1000, 10000);
          ewDecrease = false;
        }
        transitionTo(EAST_WEST_YELLOW);
      }
      break;

    case EAST_WEST_YELLOW:
      if (now - lastStateChange >= eastWestYellowTime) {
        if (priorityRequested) {
          inPriorityTransition = true;
          priorityRequested = false;
          transitionTo(ALL_RED);
        } else {
          transitionTo(NORTH_SOUTH_GREEN);
        }
      }
      break;

    case ALL_RED:
      if (inPriorityTransition && now - lastStateChange >= 3000) {
        inPriorityTransition = false;
        Serial.println("Priority transition complete. Resuming to NORTH_SOUTH_GREEN.");
        transitionTo(NORTH_SOUTH_GREEN);
      } else if (!inPriorityTransition && now - lastStateChange >= allRedTimeout) {
        Serial.println("Timeout reached in ALL_RED. Resuming to NORTH_SOUTH_GREEN.");
        transitionTo(NORTH_SOUTH_GREEN);
      }
      break;
  }
}

void TrafficLightController::reportStatus() {
  unsigned long now = millis();
  if (now - lastReportTime >= 1000) {
    lastReportTime = now;
    String report = "STATE:";
    switch (currentState) {
      case NORTH_SOUTH_GREEN: report += "NORTH_SOUTH_GREEN"; break;
      case NORTH_SOUTH_YELLOW: report += "NORTH_SOUTH_YELLOW"; break;
      case EAST_WEST_GREEN: report += "EAST_WEST_GREEN"; break;
      case EAST_WEST_YELLOW: report += "EAST_WEST_YELLOW"; break;
      case ALL_RED: report += "ALL_RED"; break;
    }
    report += ",NSG:" + String(northSouthGreenTime / 1000);
    report += ",NSY:" + String(northSouthYellowTime / 1000);
    report += ",EWG:" + String(eastWestGreenTime / 1000);
    report += ",EWY:" + String(eastWestYellowTime / 1000);
    Serial.println(report);
  }
}

void TrafficLightController::handleSerialCommand(String command) {
  command.trim();

  if (inPriorityTransition) {
    Serial.println("In priority transition. Command ignored.");
    return;
  }

  if (command == "Increase") {
    if (currentState == NORTH_SOUTH_GREEN)
      nsIncrease = true;
    else if (currentState == EAST_WEST_GREEN) 
      ewIncrease = true;
  } else if (command == "Decrease") {
      if (currentState == NORTH_SOUTH_GREEN) 
        nsDecrease = true;
      else if (currentState == EAST_WEST_GREEN) 
        ewDecrease = true;
  } else if (command == "AllRed") {
    transitionTo(ALL_RED);
  } else if (command == "Resume") {
      if (currentState == ALL_RED && !inPriorityTransition) {
        Serial.println("Received 'Resume'. Returning to NORTH_SOUTH_GREEN.");
        transitionTo(NORTH_SOUTH_GREEN);
      } else {
        Serial.println("Cannot resume: either not in ALL_RED or in priority transition.");
    }
  }
}

void TrafficLightController::transitionTo(State nextState) {
  lastStateChange = millis();
  currentState = nextState;

  switch (currentState) {
    case NORTH_SOUTH_GREEN: setNorthSouthGreen(); break;
    case NORTH_SOUTH_YELLOW: setNorthSouthYellow(); break;
    case EAST_WEST_GREEN: setEastWestGreen(); break;
    case EAST_WEST_YELLOW: setEastWestYellow(); break;
    case ALL_RED: setAllRed(); break;
  }
}

bool TrafficLightController::checkNorthSensor() {
  long d = distanceSensorNorth->measureDistanceCm();
  return (d > 0 && d < distanceThreshold);
}

bool TrafficLightController::checkSouthSensor() {
  long d = distanceSensorSouth->measureDistanceCm();
  return (d > 0 && d < distanceThreshold);
}

void TrafficLightController::setNorthSouthGreen() {
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
