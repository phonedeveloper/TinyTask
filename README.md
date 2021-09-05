# TinyTask - to replace delay(), do timeouts, etc

TinyTask allows you to schedule a function to be called at a later time.
For example, if you turn an LED on, rather than delay() until it is
time to turn it off, you can have another function to turn it off, schedule
it to run in 500 ms, and go about your way.

It works like this:

1. Create a task to run. For example, this task will turn an LED on
   if it is off, or turn it off if it is on:

```
void blinkTask() {
  static int state = LOW;
  digitalWrite(13, state = !state);
}
```

2. Create the TinyTask, and tell it what task will be run:

```
TinyTask blink(blinkTask);
```

3. Schedule the task to run every 500 ms:

```
setup() {
  ...
  blink.callEvery(500);
  ...
```

4. Check the task once every loop. If it is time to be called,
it will be called:

```
loop() {
  ...
  blink.loop();
  ...
}
```

# Full example:

```
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
```

# Create one TinyTask for each task

What it says. You can of course call the same task from different TinyTasks,
but it may be simpler to pair a task with each TinyTask.
 
# What does the TinyTask loop() function do?

NOTE: I'm going to use "blink.look()", from the example,  instead of 
just "loop()", so that it isn't confused with the main Arduino loop function.

blink.loop() does nothing more than check to see if it is
time to run the task. If it isn't time, it will exit and other things
can be done inside the main Arduino loop(). If it is time, it will call
the task and then return, and again, other things can be done inside loop().

I was thinking of calling this function
blink.checkToSeeIfItIsTimeToRunTheTaskAndIfSoCallItOtherwiseJustReturn()
but that was just too long and confusing.

So I chose the confusing name of loop() to remind people that the
Arduino loop() is a good place to call it.

Actually you can call yourTinyTask.loop() wherever you want. If you have a
really long running function, you could sprinkle calls to yourTinyTask.loop()
in there to make sure it's checked and called frequently enough (note that
this will effectively call your task from within your long running function.)
If for some reason you need your Task checker and task to run from within an
interrupt, you can call it from inside an interrupt service routine.

# TinyTask is cooperative

This means that if you have something that takes a lot of time, or you call a function that never returns, or something blocks for a long time (like a long delay(), which TinyTask is intended to replace), or your code in the main Arduino loop() does not actually loop, 
