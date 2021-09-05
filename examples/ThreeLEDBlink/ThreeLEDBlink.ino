#include "TinyTask.h"

TinyTask blinkRed(blinkRedTask);
TinyTask blinkGreen(blinkGreenTask);
TinyTask blinkYellow(blinkYellowTask);

#define RED_LED 11
#define GREEN_LED 12
#define YELLOW_LED 13

void blinkRedTask() {
  static boolean state;
  digitalWrite(RED_LED, state);
  state = !state;
}

void blinkGreenTask() {
  static boolean state;
  digitalWrite(GREEN_LED, state);
  state = !state;
}

void blinkYellowTask() {
  static boolean state;
  digitalWrite(YELLOW_LED, state);
  state = !state;
}

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  blinkRed.callEvery(50);
  blinkGreen.callEvery(250);
  blinkYellow.callEvery(1000);
}

void loop() {
  blinkRed.loop();
  blinkYellow.loop();
  blinkGreen.loop();
}
