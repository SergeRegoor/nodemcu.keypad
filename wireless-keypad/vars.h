#ifndef vars_h
#define vars_h

#include <WString.h>

class RunningVariables {
  public:
  RunningVariables(String ipAddressString, String macAddressString);
  
  void startBindingMode();
  void stopBindingMode();
  bool isInBindingMode();

  void setCheckInValues(unsigned long secondsSinceEpoch, unsigned int checkInTimeOut);
  
  String ipAddress;
  String macAddress;
  unsigned int checkInTimeOutInSeconds;
  unsigned long homeySecondsSinceEpoch;
  unsigned long lastCheckInTimer;

  private:
  int _currentMode;
};




#endif
