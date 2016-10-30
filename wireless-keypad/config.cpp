#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h>
#include "config.h"
#include "debug.h"

String getJsonValue(JsonObject* jsonObject, String key);

Configuration::Configuration(bool fromDisk) {
  if (fromDisk)
    readFromDisk();
}

bool Configuration::isComplete() {
  return (privateKey.length() > 0) && (homeyIpAddress.length() > 0) && (deviceId.length() > 0);
}

void Configuration::readFromDisk() {
  debug("Reading JSON config from disk... ");
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        int fileSize = configFile.size();
        char buffer[fileSize+1];
        for (int i = 0; i <= fileSize; i++)
          buffer[i] = '\0';
        configFile.readBytes(buffer, fileSize);
        String jsonText = buffer;
        debug("Read JSON: ");
        debugln(jsonText);
        DynamicJsonBuffer jsonBuffer;
        JsonObject* jsonObject = NULL;
        if (jsonText.length() > 0)
          jsonObject = &jsonBuffer.parseObject(jsonText);
        else
          jsonObject = &jsonBuffer.createObject();
        if (jsonObject->success()) {
          privateKey = getJsonValue(jsonObject, "privateKey");
          homeyIpAddress = getJsonValue(jsonObject, "homeyIpAddress");
          deviceId = getJsonValue(jsonObject, "deviceId");
          debugln("Config vars. Private key: \"" + privateKey + "\", Homey IP address: \"" + homeyIpAddress + "\", device ID: \"" + deviceId + "\".");
        } else debugln("Config.json contains invalid JSON.");
      } else debugln("Config.json could not be read.");
    } else debugln("Config.json not present.");
  } else debugln("SPIFFS not available.");
}

void Configuration::saveToDisk() {
  if (SPIFFS.exists("/config.json"))
    SPIFFS.remove("/config.json");
  debug("Writing JSON config to disk... ");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& jsonObject = jsonBuffer.createObject();
  jsonObject.set("privateKey", privateKey);
  jsonObject.set("homeyIpAddress", homeyIpAddress);
  jsonObject.set("deviceId", deviceId);
  File configFile = SPIFFS.open("/config.json", "w");
  jsonObject.printTo(configFile);
  configFile.close();
  debugln("Done.");
}

String getJsonValue(JsonObject* jsonObject, String key) {
  if (jsonObject == NULL) return "";
  if (key.length() == 0) return "";
  const char* value = jsonObject->get(key);
  if (value == NULL) return "";
  String stringValue = value;
  return stringValue;
}

void Configuration::clear() {
  debug("Clearing configuration... ");
  privateKey = "";
  homeyIpAddress = "";
  deviceId = "";
  saveToDisk();
  debugln("Done.");
  
  
}

