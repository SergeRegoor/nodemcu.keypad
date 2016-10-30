#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266HTTPClient.h>
#include <md5.h>
#include <math.h>
#include "homey.h"
#include "debug.h"

const unsigned int checkInIntervalInMilliSeconds = 60 * 1000;

String createHash(String ipAddress, String macAddress, String privateKey, unsigned long homeyTime);
String performHttpRequest(String ipAddress, String path);

Homey::Homey(Configuration* config, RunningVariables* runningVars, Buzzer* buzzer) {
  _config = config;
  _runningVars = runningVars;
  _buzzer = buzzer;
}

void Homey::checkIfCheckInIsNeeded() {
  if (_runningVars->isInBindingMode()) return;
  if ((_checkInTimer == 0) || ((_checkInTimer + checkInIntervalInMilliSeconds) <= millis())) { // only check every 60 seconds
    _checkInTimer = millis();
    unsigned long deviceTime = (unsigned long)round(millis() / (float)1000);
    debug("Check if check-in is needed... ");
    if ((_runningVars->lastCheckInTimer == 0) || (_runningVars->homeySecondsSinceEpoch == 0) || ((_runningVars->lastCheckInTimer + _runningVars->checkInTimeOutInSeconds) <= deviceTime)) {
      debugln("Yes, executing check-in.");
      if (_config->isComplete())
        executeCheckIn();
      else
        debugln("Not checking in with Homey, device binding not complete.");
    }
    else
      debugln("No.");
  }
}



void Homey::executeCheckIn() {
  unsigned long deviceTime = (unsigned long)round(millis() / (float)1000);
  String urlPath = "/api/app/nl.regoor.wirelesskeypad/check-in/";
  urlPath += "?deviceid=" + _config->deviceId;
  urlPath += "&devicetime=";
  urlPath += deviceTime;
  urlPath += "&hash=" + createHash(_runningVars->ipAddress, _runningVars->macAddress, _config->privateKey, deviceTime);
  String responseJsonText = performHttpRequest(_config->homeyIpAddress, urlPath);

  if (responseJsonText.length() == 0) {
    _checkInTimer = 0;
    debugln("Check-in response is empty.");
    return;
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& jsonResponse = jsonBuffer.parseObject(responseJsonText);
  if (!jsonResponse.success()) return;
  JsonObject& jsonResult = jsonResponse.get("result").as<JsonObject&>();

  bool successful = jsonResult["successful"];
  unsigned long homeySecondsSinceEpoch = jsonResult["homeySecondsSinceEpoch"];
  unsigned int checkInTimeOutInSeconds = jsonResult["checkInTimeOutInSeconds"];
  String hash = jsonResult["hash"];
  
  if (!successful) {
    _checkInTimer = 0;
    debugln("Check-in response not successful.");
    return;
  }
  if ((homeySecondsSinceEpoch <= 0) || (checkInTimeOutInSeconds <= 0)) { 
    _checkInTimer = 0;
    debugln("Check-in response does not contain seconds since epoch or check-in time-out.");
    return;
  }
  if (!isValidHash(hash, homeySecondsSinceEpoch, true)) {
    _checkInTimer = 0;
    debugln("Hash is not valid.");
    return;
  }

  _runningVars->setCheckInValues(homeySecondsSinceEpoch, checkInTimeOutInSeconds);
  debug("Check-in successful. Check-in time-out in seconds: ");
  debug(_runningVars->checkInTimeOutInSeconds);
  debug(". Homey seconds since epoch: ");
  debug(_runningVars->homeySecondsSinceEpoch);
  debugln(".");
}

void Homey::executeAccessKey(String enteredCode) {
  if (_runningVars->isInBindingMode()) return;
  if (_runningVars->homeySecondsSinceEpoch == 0) {
    debugln("Not checked-in yet, first checking in before sending access key.");
    executeCheckIn();
  }

  unsigned long deviceTime = (unsigned long)round(millis() / (float)1000);
  String urlPath = "/api/app/nl.regoor.wirelesskeypad/access-key/";
  urlPath += "?deviceid=" + _config->deviceId;
  urlPath += "&devicetime=";
  urlPath += deviceTime;
  urlPath += "&enteredcode=" + enteredCode;
  urlPath += "&hash=" + createHash(_runningVars->ipAddress, _runningVars->macAddress, _config->privateKey, deviceTime);
  String responseJsonText = performHttpRequest(_config->homeyIpAddress, urlPath);
  
  if (responseJsonText.length() == 0) {
    _buzzer->failed();
    return;
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& jsonResponse = jsonBuffer.parseObject(responseJsonText);
  if (!jsonResponse.success()) {
    _buzzer->failed();
    return;
  }
  JsonObject& jsonResult = jsonResponse.get("result").as<JsonObject&>();

  bool successful = jsonResult["successful"];
  if (successful) {
    debugln("Response successful.");
  } else {
    _buzzer->failed();
    debugln("Response not successful.");
  }
}

void Homey::executeUdpMulticast(Buzzer* buzzer) {
  debug("Sending UDP multicast. ");
  String broadcastMessage = "homey-wireless-keypad-broadcast:" + WiFi.macAddress();
  IPAddress broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();
  debug("Broadcasting message " + broadcastMessage + " to IP " + broadcastIp.toString() + "... ");
  WiFiUDP udpSocketMulticast;
  udpSocketMulticast.beginPacket(broadcastIp, 60505);
  udpSocketMulticast.write(broadcastMessage.c_str());
  udpSocketMulticast.endPacket();
  buzzer->acknowledged();
  debugln("Finished.");
}

void Homey::handleWebserverRequest(WiFiServer* webServer) {
  WiFiClient webClient = webServer->available();
  if (!webClient) return;
  debug("Web client connected, waiting for full request... ");
  unsigned long startTime = millis();
  while(!webClient.available()) {
    delay(1);
    if (millis() > (startTime + (30 * 1000)))
      break;
  }
  if (!webClient.available()) {
    debugln("Request took too long, cancelling.");
    return;
  }
  debugln("Request received.");
  DynamicJsonBuffer jsonBuffer;

  String httpRequest = webClient.readString();
  webClient.flush();
  debugln("HTTP request received: " + httpRequest);
  String jsonText = "";
  int bodyIndex = -1;
  if (httpRequest.indexOf("\r\n\r\n") >= 0)
    bodyIndex = httpRequest.indexOf("\r\n\r\n");
  else if (httpRequest.indexOf("\n\n") >= 0)
    bodyIndex = httpRequest.indexOf("\n\n");
  else
    bodyIndex = httpRequest.indexOf("\r\r");
  if (bodyIndex > 0)
    jsonText = httpRequest.substring(bodyIndex);

  if (jsonText.indexOf("{") > 0)
    jsonText.remove(0, jsonText.indexOf("{"));
  int endBracketIdx = jsonText.lastIndexOf("}");
  if ((endBracketIdx > 0) && (endBracketIdx < (jsonText.length() - 1)))
    jsonText.remove(jsonText.lastIndexOf("}")+1);
  
  debugln("JSON in body: " + jsonText);
  
  JsonObject& jsonResponse = jsonBuffer.createObject();
  jsonResponse["successful"] = false;
  jsonResponse["errorMessage"] = "Invalid request";
  jsonResponse["hash"] = "";
  if (jsonText.length() > 0) {
    JsonObject& jsonRequest = jsonBuffer.parseObject(jsonText);
    if (!jsonRequest.success())
      jsonResponse["errorMessage"] = "Invalid JSON request";
    else {
      String type = jsonRequest["type"];
      if (type.length() == 0)
        jsonResponse["errorMessage"] = "Missing type of request";
      else if (type == "bind") {
        if (!_runningVars->isInBindingMode())
          jsonResponse["errorMessage"] = "Device not in binding mode.";
        else {
          String homeyIpAddress = jsonRequest["homeyIpAddress"];
          unsigned long homeyTime = jsonRequest["homeySecondsSinceEpoch"];
          String deviceId = jsonRequest["deviceId"];
          String hash = jsonRequest["hash"];
          if (homeyIpAddress.length() == 0)
            jsonResponse["errorMessage"] = "Missing Homey IP address";
          else if (homeyTime <= 0)
            jsonResponse["errorMessage"] = "Missing Homey time.";
          else if (deviceId.length() == 0)
            jsonResponse["errorMessage"] = "Missing device ID.";
          else if (!isValidHash(hash, homeyTime, true))
            jsonResponse["errorMessage"] = "Invalid hash.";
          else {
            _config->homeyIpAddress = homeyIpAddress;
            _config->deviceId = deviceId;
            _config->saveToDisk();
            _runningVars->homeySecondsSinceEpoch = homeyTime;
            _runningVars->checkInTimeOutInSeconds = 1 * 60; // 5 minutes initially
            _runningVars->stopBindingMode();
            jsonResponse["errorMessage"] = "";
            jsonResponse["successful"] = true;
            jsonResponse["hash"] = createHash(_runningVars->ipAddress, _runningVars->macAddress, _config->privateKey, homeyTime);
          }
        }
      } else if (type == "request-check-in") {
        if (_runningVars->isInBindingMode())
          jsonResponse["errorMessage"] = "Device not in active mode.";
        else {
          unsigned long homeySecondsSinceEpoch = jsonRequest["homeySecondsSinceEpoch"];
          String hash = jsonRequest["hash"];
          if (homeySecondsSinceEpoch <= 0)
            jsonResponse["errorMessage"] = "Missing Homey time.";
          else if (!isValidHash(hash, homeySecondsSinceEpoch, true))
            jsonResponse["errorMessage"] = "Invalid hash.";
          else {
            _runningVars->homeySecondsSinceEpoch = homeySecondsSinceEpoch;
            _runningVars->lastCheckInTimer = 0; // by setting the last check to 0, we respond to the client faster, and the next iteration of the main loop will perform a new check-in
            jsonResponse["successful"] = true;
            jsonResponse["errorMessage"] = "";
            jsonResponse["hash"] = createHash(_runningVars->ipAddress, _runningVars->macAddress, _config->privateKey, homeySecondsSinceEpoch);
          }
        }
      } else if (type == "request-buzzer") {
        unsigned long homeySecondsSinceEpoch = jsonRequest["homeySecondsSinceEpoch"];
        String hash = jsonRequest["hash"];
        String buzzerType = jsonRequest["buzzer"];
        if (homeySecondsSinceEpoch <= 0)
          jsonResponse["errorMessage"] = "Missing Homey time.";
        else if (!isValidHash(hash, homeySecondsSinceEpoch, true))
          jsonResponse["errorMessage"] = "Invalid hash.";
        else {
          _buzzer->soundBuzzer(buzzerType);
          
          jsonResponse["successful"] = true;
          jsonResponse["hash"] = createHash(_runningVars->ipAddress, _runningVars->macAddress, _config->privateKey, homeySecondsSinceEpoch);
        }
      } else
        jsonResponse["errorMessage"] = "Invalid type of request";
    }
  }

  webClient.flush();
  String httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
  jsonText = "";
  jsonResponse.printTo(jsonText);
  webClient.print(httpResponse + jsonText);
  delay(1);
  debugln("Returned JSON in response: " + httpResponse);
}



bool Homey::isValidHash(String hash, unsigned long homeyTime, bool checkTime) {
  String newHash;
  if (checkTime)
    newHash = createHash(_runningVars->ipAddress, _runningVars->macAddress, _config->privateKey, homeyTime);
  else
    newHash = createHash(_runningVars->ipAddress, _runningVars->macAddress, _config->privateKey, 0);
  debugln("Comparing given hash \""+hash+"\" with new hash \""+newHash+"\".");
  return newHash == hash;
}

String createHash(String ipAddress, String macAddress, String privateKey, unsigned long homeyTime) {
  String valueToHash = macAddress + "-" + privateKey + "-" + ipAddress + "-$up3rS3cr3t!-" + homeyTime;
  MD5Builder md5;
  md5.begin();
  md5.add(valueToHash);
  md5.calculate();
  return md5.toString();
}

String performHttpRequest(String ipAddress, String path) {
  debug("Performing HTTP request to ");
  debug(ipAddress);
  debug(" ");
  debug(path);
  debugln(".");
  HTTPClient httpClient;
  httpClient.begin(ipAddress.c_str(), 80, path.c_str());
  auto httpCode = httpClient.GET();
  if (httpCode == 200) {
    String response = httpClient.getString();
    debug("HTTP request succcessful, received content: ");
    debugln(response);
    return response;
  }
  httpClient.end();
  debug("HTTP request failed, received HTTP code ");
  debugln(httpCode);
  return "";
}
