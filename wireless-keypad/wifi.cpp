#include <ESP8266WiFi.h>
#include "wifi.h"
#include "debug.h"

Wifi::Wifi() {
  _wifiManager = new WiFiManager();
}

void Wifi::initialize(String ssidName, Configuration* config) {
  _wifiManager->setConnectTimeout(60);
  WiFiManagerParameter privateKeyParameter("privateKey", "Private key in Homey app settings", config->privateKey.c_str(), 50);
  _wifiManager->addParameter(&privateKeyParameter);

  if (config->privateKey.length() > 0)
    _wifiManager->autoConnect(ssidName.c_str());
  else
    _wifiManager->startConfigPortal(ssidName.c_str());

  String newPrivateKey = privateKeyParameter.getValue();
  if (config->privateKey != newPrivateKey) {
    debugln("Configured private key is new or different, saving to disk.");
    config->privateKey = newPrivateKey;
    config->saveToDisk();
  }

  debug("Waiting for WiFi connection... ");
  while (WiFi.status() != WL_CONNECTED)
    delay(500);
  debug("Done. IP address: ");
  debug(WiFi.localIP().toString());
  debug(". Subnet: ");
  debug(WiFi.subnetMask().toString());
  debug(". Gateway: ");
  debug(WiFi.gatewayIP().toString());
  debugln(".");
}

void Wifi::resetSettings() {
  debug("Clearing wifi settings... ");
  _wifiManager->resetSettings();
  debugln("Done.");
}

