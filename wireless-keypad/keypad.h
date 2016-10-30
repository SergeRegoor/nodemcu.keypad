#ifndef keypad_h
#define keypad_h

#include <WString.h>

#include "buzzer.h"
void checkKeypadInputTimeOut(Buzzer* buzzer);
String getEnteredKeypadCode(Buzzer* buzzer, bool inBindingMode);
void resetKeypad();

#endif
