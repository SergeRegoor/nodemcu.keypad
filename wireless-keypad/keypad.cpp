#include <ESP8266WiFi.h>
#include <Keypad.h>
#include "keypad.h"
#include "debug.h"

const byte KEYPAD_ROWS = 4; //four rows
const byte KEYPAD_COLS = 4; //four columns
byte rowPins[KEYPAD_ROWS] = {D7, D6, D5, D4}; //connect to the row pinouts of the keypad
byte colPins[KEYPAD_COLS] = {D3, D2, D1, D0}; //connect to the column pinouts of the keypad
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
Keypad _keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);


String _buffer = "";
String _enteredCode = "";
unsigned long _mostRecentKeyPress = 0;
unsigned int _keyTimeoutInSeconds = 3;
unsigned int _codeLength = 5;


void checkKeypadInputTimeOut(Buzzer* buzzer) {
  if ((_mostRecentKeyPress > 0) && ((_mostRecentKeyPress + (_keyTimeoutInSeconds * 1000)) < millis())) {
    debugln("Key press time-out.");
    resetKeypad();
    buzzer->failed();
  }
}

String getEnteredKeypadCode(Buzzer* buzzer, bool inBindingMode) {
  char pressedKey = _keypad.getKey();
  if (pressedKey != NO_KEY) {
    if (pressedKey == '#') {
      _mostRecentKeyPress = 0;
      if (_buffer.length() == _codeLength) {
        _enteredCode = _buffer;
        _buffer = "";
        debug("Entered code: ");
        debugln(_enteredCode);
      } else if (inBindingMode && (_buffer.length() == 0)) {
        _enteredCode = "#";
        _buffer = "";
      } else {
        resetKeypad();
        buzzer->failed();
      }
    } else {
      if ((_buffer.length() + 1) > _codeLength) {
        resetKeypad();
        buzzer->failed();
      } else
        _buffer += pressedKey;
        buzzer->keyPressed();
        _mostRecentKeyPress = millis();
    }
  }
  return _enteredCode;
}


void resetKeypad() {
  debugln("Resetting entered code.");
  _buffer = "";
  _enteredCode = "";
  _mostRecentKeyPress = 0;
}

