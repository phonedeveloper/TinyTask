#include "TinyTask.h"

#define RED_LED 13
#define GREEN_LED 12
#define YELLOW_LED 11

typedef struct LEDInfo {
  int pin;
  boolean state;
} LEDInfo;

void blinkTask(void* ledParam) {
  LEDInfo* led = (LEDInfo*)ledParam;    // copy the pointer from a void* type to a LEDInfo* type so we don't have to cast every usage of it!
  
  digitalWrite(led->pin, led->state);
  led->state = !led->state;
}

LEDInfo redLED = {11, false};
LEDInfo greenLED = {12, false};
LEDInfo yellowLED = {13, false};

TinyTask blinkRed(blinkTask);
TinyTask blinkGreen(blinkTask);
TinyTask blinkYellow(blinkTask);

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  blinkRed.callEvery(50, (void*) &redLED);
  blinkGreen.callEvery(250, (void*) &greenLED);
  blinkYellow.callEvery(1000, (void*) &yellowLED);
}

void loop() {
  blinkRed.loop();
  blinkGreen.loop();
  blinkYellow.loop();
}
