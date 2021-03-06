# TinyTask for Arduino: create non-blocking delays, do timeouts, etc

***This new library is*** **ALPHA** ***and needs more testing. Please report any issues. Thanks!***

**CURRENT STATUS**
TinyTask v0.0.1 is a fail except for recurring events started when the library is first used. Issues 2 and 3 will address this. The branch "fix-issues-2-3" currently contains proposed fixes for these issues. It will be merged once unit tests are created and the fixes pass them. (See issue #1 regarding unit testing.) Once these fixes are confirmed, v0.0.2 will be released.

TinyTask allows you to schedule a function to be called at a later time.
For example, rather than ```delay()``` to flash an LED, which prevents you from
doing anything else, you can create a function that toggles the LED, and then
use TinyTask to call it periodically, letting you do other things in between.

## How it works

1. Create a task (a function) to run. For example, this task will turn an LED on
   if it is off, or turn it off if it is on:

```
void blinkTask() {
  static int state = LOW;
  digitalWrite(13, state = !state);
}
```

2. Name the TinyTask, and tell it what task it will run. Here we create a TinyTask named "blink" that will call ```blinkTask()``` above:

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
it will be called from ```blink.loop()```. Otherwise it exits and you can do other stuff in the meantime:

```
loop() {
  ...
  blink.loop();
  ...
}
```

## Full example

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

## CallEvery, CallAt, or CallIn

```callEvery(long interval)``` calls the task every at specified interval.
```callIn(long interval)``` calls the task once after interval milliseconds (or microseconds).
```callAt(unsigned long futureTime)``` calls the task at a future time (as compared to millis() or micros()).

Each returns ```true``` if the task was scheduled. You'll see ```false``` only if a negative value was supplied for ```CallEvery()``` or ```CallIn()```. ```CallAt()``` will return false if the future time is more than 2147483647 milliseconds or microseconds from now (depending on what timebase you are using).

The maximum time ahead that can be scheduled / maximum interval is **24.8 days** (default/using milliseconds) or **35.7 minutes** (using microseconds). The corresponding max value for ```callEvery()``` or ```callIn()``` is **2147483647** (2^31 - 1).

## Milliseconds or microseconds

TinyTask times are in milliseconds by default, compared to the current Arduino time reported by the Arduino ```millis()``` function.

If microseconds are desired, the library's ```useMicros()``` method will switch the timebase to microseconds (using Arduino ```micros()```) for that TinyTask. Use ```useMillis()``` to switch back to milliseconds.

## Create one TinyTask for each task

What it says. You can of course call the same task from different TinyTasks,
but it may be simpler to pair a task with each TinyTask.
 
## What does the TinyTask loop() function do?

I'm going to refer to the example use of the ```loop()``` function, ```blink.look()```, instead of 
just ```loop()```, so that it isn't confused with the main Arduino ```loop()``` function.

```blink.loop()``` does nothing more than check to see if it is
time to run the task, and run it if it is. If it isn't time, it will exit and other things
can be done inside the main Arduino ```loop()```. If it is time, it will call
the task and then return, and again, other things can be done inside loop().

I was thinking of calling this function
```blink.checkToSeeIfItIsTimeToRunTheTaskAndIfSoCallItOtherwiseJustReturn()```
but that was just too long and confusing.

So I chose the less confusing name of ```loop()``` to remind people that the
Arduino ```loop()``` is a good place to call it.

Actually you can call ```blink.loop()``` wherever you want. If you have a
really long running function, you could sprinkle calls to ```blink.loop()```
in there to make sure it's checked and called frequently enough (note that
this will effectively call your task from within your long running function if it is time.)

## TinyTask is cooperative

This means that if you have something that takes a lot of time, or you call a function that never returns, or something blocks for a long time (like a long ```delay()```, which TinyTask is intended to replace), or your code in the main Arduino ```loop()``` does not actually loop, your task won't get called. Since TinyTask's loop() function calls the task, if other code is running when it's time to call your task, it won't get called until that code is finished and TinyTask's loop() has a chance to run.

## Releases

Latest: [v0.0.1](https://github.com/phonedeveloper/TinyTask/releases/tag/v0.0.1):
* 2021-09-05
* Pre-release
* Only ```callEvery()``` works, and must be called before first call to TinyTask's ```loop()``` (see [issue #2](https://github.com/phonedeveloper/TinyTask/issues/2), [issue #3](https://github.com/phonedeveloper/TinyTask/issues/3) against this version)

## Version info

**0.0.1 - 2021/09/05**: initial pre-release
