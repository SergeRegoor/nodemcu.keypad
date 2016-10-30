#ifndef wifi_h
#define wifi_h

#include <WiFiManager.h>
#include <WString.h>
#include "config.h"

class Wifi {
  public:
  Wifi();

  void initialize(String ssidName, Configuration* config);
  void resetSettings();

  private:
  WiFiManager* _wifiManager;
};


#endif
