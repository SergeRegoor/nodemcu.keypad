#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include "homey.h"
#include "wifi.h"
#include "keypad.h"
#include "debug.h"

Homey* _homey;
Configuration* _config;
RunningVariables* _runningVars;
Buzzer* _buzzer;
Wifi* _wifi;
WiFiServer* _webServer;

String resetCode = "73738"; // reset 
String bindCode  = "46639"; // homey --> bind
unsigned long bindingBroadcastTimer = 0;

const unsigned int udpBroadcastIntervalInMilliSeconds = 3000;

void setup() {
  Serial.begin(115200);
  debugln("");
  debugln("Setting up Wireless Entry device...");

  _buzzer = new Buzzer();
  _buzzer->keyPressed();
  _config = new Configuration(true);
  _wifi = new Wifi();
  _wifi->initialize("Wireless Keypad config", _config);
  _runningVars = new RunningVariables(WiFi.localIP().toString(), WiFi.macAddress());
  _homey = new Homey(_config, _runningVars, _buzzer);
  _webServer = new WiFiServer(60506);
  _webServer->begin();
  
  _buzzer->acknowledged();
  delay(350);
  _buzzer->keyPressed();
}


void loop() {
  _homey->handleWebserverRequest(_webServer);

  checkKeypadInputTimeOut(_buzzer);
  
  String enteredCode = getEnteredKeypadCode(_buzzer, _runningVars->isInBindingMode());
  
  // Run either binding or active mode
  if (_runningVars->isInBindingMode())
    executeBindingMode(enteredCode);
  else
    executeActiveMode(enteredCode);
}


void executeActiveMode(String enteredCode) {
  if (enteredCode.length() > 0) {
    debugln("Executing active mode for entered code.");
    // If a code has been entered, check what to do.
    if (enteredCode == bindCode) {
      // The binding code has been entered, start the binding mode
      debugln("Start binding mode...");
      bindingBroadcastTimer = 0;
      _runningVars->startBindingMode();
    } else if (enteredCode == resetCode) {
      // The reset code has been entered, reset the device
      debugln("Forcing reset...");
      _config->clear();
      _wifi->resetSettings();
      ESP.restart();
    } else {
      // Send the entered key to Homey
      if (_config->isComplete())
        _homey->executeAccessKey(enteredCode);
      else
        debugln("Not sending access key, device binding not complete.");
    }

    // Reset entered code to start over with the keypad sequence
    resetKeypad();
  }

  _homey->checkIfCheckInIsNeeded();
}


void executeBindingMode(String enteredCode) {
  if (enteredCode == "#") {
    // If the "#" key is pressed, cancel the binding mode.
    debugln("Stop binding mode.");
    _runningVars->stopBindingMode();
    _buzzer->failed();
  } else if ((bindingBroadcastTimer == 0) || ((bindingBroadcastTimer + udpBroadcastIntervalInMilliSeconds) < millis())) {
    // Broadcast a UDP message every 3 seconds
    _homey->executeUdpMulticast(_buzzer);
    bindingBroadcastTimer = millis();
  }
  if (enteredCode.length() > 0)
    resetKeypad();
}





