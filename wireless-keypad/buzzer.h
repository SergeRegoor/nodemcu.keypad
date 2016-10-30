#ifndef buzzer_h
#define buzzer_h

class Buzzer {
  public:
  Buzzer();

  void armed();
  void disarmed();
  void acknowledged();
  void failed();
  void keyPressed();

  void soundBuzzer(String buzzerType);

  private:
  void fireBuzzer(int buzzerMode);
};





#endif
