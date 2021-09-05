#include "TinyTask.h"

void blinkTask() {          //  <-- The task to be run
  static int state = LOW;
  digitalWrite(13, state = !state);
}

TinyTask blink(blinkTask);  //  <-- Creates a TinyTask that will call the task

void setup() {
  pinMode(13, OUTPUT);
  blink.callEvery(250);     //  <-- Call the task every 250 ms
}

void loop() {
  // do other stuff
  blink.loop();             //  <-- Check to see if time to run task. If so, run it.
  // do other stuff
}
