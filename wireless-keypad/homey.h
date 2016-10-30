#ifndef homey_h
#define homey_h

#include <ESP8266WiFi.h>
#include "config.h"
#include "vars.h"
#include "buzzer.h"

class Homey {
  public:
  Homey(Configuration* config, RunningVariables* runningVars, Buzzer* buzzer);
  
  void checkIfCheckInIsNeeded();
  void executeCheckIn();
  void executeAccessKey(String enteredCode);
  void executeUdpMulticast(Buzzer* buzzer);
  void handleWebserverRequest(WiFiServer* webServer);
  
  private:
  Configuration* _config;
  RunningVariables* _runningVars;
  Buzzer* _buzzer;
  bool isValidHash(String hash, unsigned long homeyTime, bool checkTime);
  unsigned long _checkInTimer;
};





#endif
