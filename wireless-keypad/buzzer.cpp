#include <ESP8266WiFi.h>
#include "buzzer.h"

const int buzzerPin = 15; // physically D8 on NodeMCU

const unsigned int BUZZER_ARMED         = 1;
const unsigned int BUZZER_DISARMED      = 2;
const unsigned int BUZZER_ACKNOWLEDGED  = 3;
const unsigned int BUZZER_FAILURE       = 4;
const unsigned int BUZZER_KEYPRESS      = 6;

Buzzer::Buzzer() {
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
};

void Buzzer::armed() {
  fireBuzzer(BUZZER_ARMED);
}

void Buzzer::disarmed() {
  fireBuzzer(BUZZER_DISARMED);
}

void Buzzer::acknowledged() {
  fireBuzzer(BUZZER_ACKNOWLEDGED);
}

void Buzzer::failed() {
  fireBuzzer(BUZZER_FAILURE);
}

void Buzzer::keyPressed() {
  fireBuzzer(BUZZER_KEYPRESS);
}

void Buzzer::soundBuzzer(String buzzerType) {
  unsigned int buzzer = BUZZER_KEYPRESS;
  if (buzzerType == "armed")
    buzzer = BUZZER_ARMED;
  else if (buzzerType == "disarmed")
    buzzer = BUZZER_DISARMED;
  else if (buzzerType == "acknowledged")
    buzzer = BUZZER_ACKNOWLEDGED;
  else if (buzzerType == "failed")
    buzzer = BUZZER_FAILURE;
  else if (buzzerType == "keypressed")
    buzzer = BUZZER_KEYPRESS;
  fireBuzzer(buzzer);
}

void Buzzer::fireBuzzer(int buzzerMode) {
  if (buzzerMode == BUZZER_ARMED) {
    digitalWrite(buzzerPin, HIGH);
    delay(5000);
    digitalWrite(buzzerPin, LOW);
  } else if (buzzerMode == BUZZER_DISARMED) {
    for (int i = 0; i < 5; i++) {
      if (i > 0)
        delay(500);
      digitalWrite(buzzerPin, HIGH);
      delay(1000);
      digitalWrite(buzzerPin, LOW);
    }
  } else if (buzzerMode == BUZZER_ACKNOWLEDGED) {
    digitalWrite(buzzerPin, HIGH);
    delay(350);
    digitalWrite(buzzerPin, LOW);
  } else if (buzzerMode == BUZZER_FAILURE) {
    for (int i = 0; i < 3; i++) {
      if (i > 0)
        delay(100);
      digitalWrite(buzzerPin, HIGH);
      delay(100);
      digitalWrite(buzzerPin, LOW);
    }
  } else if (buzzerMode == BUZZER_KEYPRESS) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
  }
};

