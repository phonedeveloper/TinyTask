/*
 * TinyTask.cpp - Library for scheduling a function to run at a future time.
 * This can be used for things like blinking an LED, timeouts, delays, and other stuff
 * No threads are used. Requires millis() or micros() to work but uses no other Timers.
 * 
 * Most sketches will only need to call callEvery(), callIn() or callAt(), and loop().
 * 
 * EXAMPLE sketch:

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

 * Usage (for tasks taking no parameters):
 * 
 * - Create a task: a function with no parameters that returns void that will be run later
 * - Instantiate a TinyTask that calls this function
 * - Use callAt(), callIni() or callEveryi() to specify when the function should be called:
 *   callIn() calls a function x millseconds or microseconds from now
 *   callAt() calls a function at the time provided (must be within 31 bits of the current time
 *   callEvery() repeatedly calls a function at the supplied interval
 * - Put loop into the Arduino loop() method to check and call the function when it is time
 *   loop checks to see if it is time to run the function, and then runs it.
 *   
 * A TinyTask kinda looks like a thread but isn't. Since the task is called from loop, 
 * and not an interrupt, there is no reentrance concern (unless you use interrupts that use 
 * the TinyTask library, or data that a task also uses.)
 * 
 * There are three more functions that could be useful:
 * - useMillis() or useMicros() sets the timebase (default milliseconds)
 * - cancel() allows you to cancel a task scheduled to be run
 *   cancel() can be used to stop a callEvery task that repeats.
 * - remaining() tells you if the task is going to run and when.
 *   remaining() returns the amount of time (millis or micros) remaining, or -1 if not scheduled.
 * 
 * TinyTask tasks cannot take parameters, except one exception: support is provided for a task
 * that can take a single pointer as a parameter. This open doors, but it's use is
 * kinda complicated and should be ignored unless you are desperate for this capability;
 * you might try using globals or start thinking about another task library. If you must do this,
 * your task must take a void* parameter, convert it internally, and pass the parameters in the
 * callAt, callIn or callEvery method. See TinyTaskBlinkThreeLEDExample. Passing a pointer allows
 * the task to receive a pointer to a structure containing data, pointers, even function pointers 
 * that can be changed up until the moment it is called, and then can change anything inside that
 * structure before it returns.
 * 
 * Created by Sean Sheedy, Sunday September 5, 2021

Copyright (c) 2021 Sean Sheedy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/


#include "Arduino.h"
#include "TinyTask.h"

// TODO: Better support for tasks with arguments. I'm not sure this can be done without using
//       variadic functions, macros and limiting the number of arguments

// The 32 bit millis() and micros() roll over after 49.7 days and 1 hr 11 minutes respectively.
// The timeout expiration subtracts the current time from the future timeout and if the 32nd bit is set,
// it decides time has expired (this bit is set when the current time advances past the timeout time.)
// This is an easy calculation if the result is cast to (long); we can just use the sign of the subtraction
// for the test (greater or less than zero). However, it also means we can only have a timeout up to 31 bits,
// or half of the rollover values. It's possible, of course, to write the library so that timeouts are
// longer, but it also means that if loop() is delayed, it can be delayed too much and cause the timeout to
// "unexpire". 31 bits provides equal time for timeout expiration and opportunity to service a timeout.

// Negative or delay/interval values cause callIn and callEvery to fail. In most cases these
// would be interpreted as the timeout having passed, and the task run immediately. However,
// because of how timeout is determined (see above), 
// if the negative value is very large, and enough time passes between it is set and loop() is
// called, it may roll over and become a very large delay. To prevent this, we simply reject
// negative values.

// callAt requires a future expiration time as a parameter. This must be no more than 31 bits
// away from the current time (the result of future expiration minus current time, cast to a long, 
// must be positive). This is required to adhere to how timeouts are calculated, described above.
// The call will fail if a longer timeout is submitted.

// Tasks to be called must return void. As for parameters, the simplest thing is for the task to
// not take any parameters if you don't have to. The next is to supply the task with one
// parameter which can be appended to the callIn/At/Every call. If this is done, the task must
// be created with the number of arguments appended to the TinyTask constructor. More parameters
// are not supported because of the additional code required.

TinyTask::TinyTask(TaskToCall taskToCall) : 
  taskToCall(taskToCall) {
    TinyTask::taskToCallTakesPtr = NULL;
}

TinyTask::TinyTask(TaskToCallTakesPtr taskToCallTakesPtr) : 
  taskToCallTakesPtr(taskToCallTakesPtr) {
    TinyTask::taskToCall = NULL;
}

boolean TinyTask::callIn(long interval, void* pointerParam) {
  TinyTask::pointerParam = pointerParam;
  callIn(interval);
}

boolean TinyTask::callIn(long interval) {
  if (TinyTask::interval < 0) return false;    // eliminates race condition: a very large negative number which may delay a long time or run immediately
  if (TinyTask::microseconds) {
    TinyTask::timeout = micros() + interval;   // calculate the time in the future this will run
  } else {
    TinyTask::timeout = millis() + interval;
  }
  TinyTask::periodic = false;
  TinyTask::active = true;
  return true;
}

boolean TinyTask::callAt(unsigned long futureTime, void* pointerParam) {
  TinyTask::pointerParam = pointerParam;
  callAt(futureTime);
}

boolean TinyTask::callAt(unsigned long futureTime) {
  if (TinyTask::microseconds) {
    if (futureTime - micros() < 0) {    // if too far into the future/in the past, reject the request
      return false;
    }
  } else {
    if (futureTime - millis() < 0) {    // if too far into the future/in the past, reject the request
      return false;
    }
  }
  TinyTask::timeout = futureTime;
  TinyTask::periodic = false;
  TinyTask::active = true;
  return true;
}

boolean TinyTask::callEvery(long interval, void* pointerParam) {
  TinyTask::pointerParam = pointerParam;
  callEvery(interval);
}

boolean TinyTask::callEvery(long interval) {
  if (interval < 0) return false;   // do not permit intervals more than 
  TinyTask::interval = interval;
  if (TinyTask::microseconds) { 
    TinyTask::timeout = micros() + interval;
  } else {
    TinyTask::timeout = millis() + interval;
  }
  TinyTask::periodic = true;
  TinyTask::active = true;
  return true;
}

void TinyTask::loop() {
  if (TinyTask::active) {
    if (TinyTask::remaining() <= 0) {
      if (TinyTask::periodic) {
        while (TinyTask::remaining() <= 0) {
          TinyTask::timeout = TinyTask::timeout + TinyTask::interval;
        }
      } else {
        TinyTask::active = false;
      }
      TinyTask::callTask();
    }
  }
}

void TinyTask::callTask() {
  if (TinyTask::taskToCall != NULL) {
    TinyTask::taskToCall();
  } else if (TinyTask::taskToCallTakesPtr != NULL) {
    TinyTask::taskToCallTakesPtr(TinyTask::pointerParam);
  }
}

void TinyTask::useMillis() {
  TinyTask::microseconds = false;
}

void TinyTask::useMicros() {
  TinyTask::microseconds = true;
}

/*
 * Tip: Use this to find out how long the processor can sleep before the next task is due.
 * If you have multiple TinyTasks, check all to find the shortest time to sleep.
 */
long TinyTask::remaining() {
  if (!TinyTask::active) return -1L;
  else {
    unsigned long timeLeft;
    if (TinyTask::microseconds) {
      timeLeft = TinyTask::timeout - micros();      
    } else {
      timeLeft = TinyTask::timeout - millis();
    }
    if (timeLeft < 0) {
      return 0;
    } else {
      return timeLeft;
    }
  }
}

void TinyTask::cancel() {
  TinyTask::active = false;
}
