#include <ESP8266WiFi.h>

void debug(char* text) {
    Serial.print(text);
}
void debugln(char* text) {
    Serial.println(text);
}

void debug(String text) {
    Serial.print(text);
}
void debugln(String text) {
    Serial.println(text);
}

void debug(long text) {
    Serial.print(text);
}
void debugln(long text) {
    Serial.println(text);
}

