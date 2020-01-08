/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/home/particle/liftsensor/firmware/electron/src/main.ino"
#include "Particle.h"
#include "math.h"

#include "Battery.h"
#include "SensorDriver.h"
#include "NetworkDriver.h"
#include "RootController.h"
#include "Debugging.h"

void setup();
void loop();
#line 10 "/home/particle/liftsensor/firmware/electron/src/main.ino"
const time_t mainLoopDelay = 10;


Battery battery = Battery();
SensorDriver sensorDriver = SensorDriver();
NetworkDriver networkDriver = NetworkDriver();
RootController rootController = RootController(battery, sensorDriver, networkDriver);

void setup() {
    Serial.begin(115200);
    Debugging::setup();
    Particle.variable("loopDelay", mainLoopDelay);
}


void loop() {
    Debugging::loop();
    delay(mainLoopDelay);
    rootController.process();
}
