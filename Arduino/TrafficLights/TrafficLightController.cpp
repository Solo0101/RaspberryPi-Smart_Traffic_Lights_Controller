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

  // Update sensor hold status
  if (!emergencyActive) {
    sensorHoldActive = (checkNorthSensor() || checkSouthSensor());
  } else {
    sensorHoldActive = false;
  }

  switch (currentState) {

    case NORTH_SOUTH_GREEN:
      if (emergencyActive) {
        Serial.println("[FSM] Emergency detected -> switching to ALL_RED.");
        transitionTo(ALL_RED);
        break;
      }

      if (sensorHoldActive) {
        // Stay in NORTH_SOUTH_GREEN while vehicle detected
        break;
      }

      if (now - lastStateChange >= northSouthGreenTime) {
        nsAdjusted = false;  // Reset adjustment flag for next cycle
        ewAdjusted = false;
        transitionTo(NORTH_SOUTH_YELLOW);
      }
      break;

    case NORTH_SOUTH_YELLOW:
      if (now - lastStateChange >= northSouthYellowTime) {
        nsAdjusted = false;
        ewAdjusted = false;
        transitionTo(EAST_WEST_GREEN);
      }
      break;

    case EAST_WEST_GREEN:
      if (emergencyActive) {
        Serial.println("[FSM] Emergency detected -> switching to ALL_RED.");
        transitionTo(ALL_RED);
        break;
      }

      if (now - lastStateChange >= eastWestGreenTime) {
        nsAdjusted = false;
        ewAdjusted = false;  // Reset adjustment flag for next cycle
        transitionTo(EAST_WEST_YELLOW);
      }
      break;

    case EAST_WEST_YELLOW:
      if (now - lastStateChange >= eastWestYellowTime) {
        nsAdjusted = false;
        ewAdjusted = false;
        transitionTo(NORTH_SOUTH_GREEN);
      }
      break;

    case ALL_RED:
      if (now - allRedStartTime >= allRedTimeout) {
        if (emergencyActive) {
          emergencyActive = false;
          Serial.println("[FSM] Emergency timeout expired. Resuming to NS_GREEN.");
          nsAdjusted = false;
          ewAdjusted = false;
          transitionTo(NORTH_SOUTH_GREEN);
        }
      }
      break;
  }
}


void TrafficLightController::reportStatus() {
  unsigned long now = millis();
  if (now - lastReportTime >= reporTimeInterval) {
    lastReportTime = now;
    String report = "STATE:";
    switch (currentState) {
      case NORTH_SOUTH_GREEN: report += "NORTH_SOUTH_GREEN"; break;
      case NORTH_SOUTH_YELLOW: report += "NORTH_SOUTH_YELLOW"; break;
      case EAST_WEST_GREEN: report += "EAST_WEST_GREEN"; break;
      case EAST_WEST_YELLOW: report += "EAST_WEST_YELLOW"; break;
      case ALL_RED: report += "ALL_RED"; break;
    }
    report += ",NSG:" + String(northSouthGreenTime / milisecondsInSecond);
    report += ",NSY:" + String(northSouthYellowTime / milisecondsInSecond);
    report += ",EWG:" + String(eastWestGreenTime / milisecondsInSecond);
    report += ",EWY:" + String(eastWestYellowTime / milisecondsInSecond);
    Serial.println(report);
  }
}

void TrafficLightController::handleSerialCommand(String command) {
  command.trim();

  if (command == "AllRed") {
    if (!emergencyActive) {
      emergencyActive = true;
      Serial.println("[FSM] Emergency mode activated -> switching to ALL_RED.");
      transitionTo(ALL_RED);
      allRedStartTime = millis();  // Start timeout tracking
    } else {
      Serial.println("[FSM] Emergency already active. Ignoring redundant AllRed.");
    }
  }

  else if (command == "Resume") {
    if (emergencyActive && currentState == ALL_RED) {
      emergencyActive = false;
      Serial.println("[FSM] Emergency cleared. Resuming to NORTH_SOUTH_GREEN.");
      transitionTo(NORTH_SOUTH_GREEN);
    } else {
      Serial.println("[FSM] Resume ignored.");
    }
  }

  else if (currentState != ALL_RED && !sensorHoldActive) {
    if (command == "IncreaseNS" && !nsAdjusted) {
      northSouthGreenTime = min(northSouthGreenTime + 1000, 60000);
      nsAdjusted = true;
      Serial.println("[FSM] Increased NS_GREEN duration.");
    } else if (command == "DecreaseNS" && !nsAdjusted) {
      northSouthGreenTime = max(northSouthGreenTime - 1000, 10000);
      nsAdjusted = true;
      Serial.println("[FSM] Decreased NS_GREEN duration.");
    } else if (command == "IncreaseEW" && !ewAdjusted) {
      eastWestGreenTime = min(eastWestGreenTime + 1000, 60000);
      ewAdjusted = true;
      Serial.println("[FSM] Increased EW_GREEN duration.");
    } else if (command == "DecreaseEW" && !ewAdjusted) {
      eastWestGreenTime = max(eastWestGreenTime - 1000, 10000);
      ewAdjusted = true;
      Serial.println("[FSM] Decreased EW_GREEN duration.");
    } else {
      Serial.println("[FSM] Adjustment ignored (already applied).");
    }
  } else {
    Serial.println("[FSM] Adjustment command ignored: in ALL_RED or sensor holding.");
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
