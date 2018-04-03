#include "Particle.h"
#include "math.h"

#include "Battery.h"
#include "SensorDriver.h"
#include "RootController.h"

const time_t mainLoopDelay = 100;

Battery battery = Battery();
SensorDriver sensorDriver = SensorDriver();
RootController rootController = RootController(battery, sensorDriver);

void setup() {
    Serial.begin(115200);
    Particle.variable("loopDelay", mainLoopDelay);
}


void loop() {
    delay(mainLoopDelay);
    rootController.process();
}
