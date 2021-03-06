#include "Particle.h"
#include "math.h"

#include "Battery.h"
#include "SensorDriver.h"
#include "NetworkDriver.h"
#include "RootController.h"
#include "Debugging.h"

const time_t mainLoopDelay = 10;

PRODUCT_ID(7299);
PRODUCT_VERSION(3);

// PMIC().disableCharging();
Battery battery = Battery();
SensorDriver sensorDriver = SensorDriver();
NetworkDriver networkDriver = NetworkDriver();
RootController rootController = RootController(battery, sensorDriver, networkDriver);

void setup() {
    Serial.begin(115200);
    Debugging::setup();
    Particle.variable("loopDelay", mainLoopDelay);
    rootController.init();
}


void loop() {
    Debugging::loop();
    delay(mainLoopDelay);
    rootController.process();
}
