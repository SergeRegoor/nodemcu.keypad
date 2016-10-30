#include <ESP8266WiFi.h>
#include "vars.h"

unsigned int MODE_ACTIVE      = 1;
unsigned int MODE_BINDING     = 2;

RunningVariables::RunningVariables(String ipAddressString, String macAddressString) {
  _currentMode = MODE_ACTIVE;
  ipAddress = ipAddressString;
  macAddress = macAddressString;
  checkInTimeOutInSeconds = 0;
  homeySecondsSinceEpoch = 0;
  lastCheckInTimer = 0;
}

void RunningVariables::startBindingMode() {
  _currentMode = MODE_BINDING;
}

void RunningVariables::stopBindingMode() {
  _currentMode = MODE_ACTIVE;
}

bool RunningVariables::isInBindingMode() {
  return _currentMode == MODE_BINDING;
}

void RunningVariables::setCheckInValues(unsigned long secondsSinceEpoch, unsigned int checkInTimeOut) {
  homeySecondsSinceEpoch = secondsSinceEpoch;
  checkInTimeOutInSeconds = checkInTimeOut;
  lastCheckInTimer = (unsigned long)round(millis() / (float)1000);
}

