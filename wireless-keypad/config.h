#ifndef config_h
#define config_h

class Configuration {
  public:
  Configuration(bool fromDisk);
  String privateKey;
  String homeyIpAddress;
  String deviceId;
  
  bool isComplete();
  void readFromDisk();
  void saveToDisk();
  void clear();
};




#endif
