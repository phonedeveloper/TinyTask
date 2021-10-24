#ifndef TinyTask_h
#define TinyTask_h

#include "Arduino.h"

typedef void (*TaskToCall)(void);             // defines a callback function datatype
typedef void (*TaskToCallTakesPtr)(void*);    // defines a callback function that takes a pointer

class TinyTask {

  private:
  
    bool periodic;                            // signals that callEvery() established a recurring task
    bool active = false;                               // signals that the task is currently pending
    bool microseconds = false;                // indicates whether or not micros() instead of millis() is used
    void* pointerParam;                       // the pointer parameter to supply to the callback
    long interval;                            // for tasks started with callEvery(), the interval between calls
    unsigned long timeout;                    // the next time a task should be called
    TaskToCall taskToCall = NULL;             // the function that will be called
    TaskToCallTakesPtr taskToCallTakesPtr = NULL;  // the function with pointer parameter that will be called
    void callTask();                          // calls task, with arguments if provided

  public:
  
    TinyTask(TaskToCall taskToCall);          // optionally specify task type
    TinyTask(TaskToCallTakesPtr taskToCallTakesPtr);   // optionally specify task type
    boolean callIn(long interval, void* pointerParam);  // task to run interval millis or micros, that takes a pointer
    boolean callIn(long interval);            // sets task to run delay millis or micros from now
    boolean callAt(unsigned long futureTime, void* pointerParam);  // task to run interval millis or micros, that takes a pointer
    boolean callAt(unsigned long futureTime); // sets task to run at a specific time in millis or micros
    boolean callEvery(long period, void* pointerParam);      // sets task to run every period millis or micros
    boolean callEvery(long period);           // sets task to run every period millis or micros
    void useMicros();                         // used to select micros() as time base
    void useMillis();                         // used to select millis() as time base (default)
    long remaining();                         // used to see how much time is remaining before next call
    void cancel();                            // stops the task from running in the future
    void loop();                              // call in a loop to check if time to run task
    
};

#endif
