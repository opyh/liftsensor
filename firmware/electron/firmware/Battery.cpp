#include "Battery.h"

void Battery::init() {
    Particle.variable("charge", stateOfCharge);
}

void Battery::process(time_t now) {
    if ((now - lastBatteryStateSentTimeStamp) > batteryTimeInterval) {
        stateOfCharge = fuel.getSoC();
        Particle.publish("charge", String(fuel.getSoC()), 3600, PRIVATE);
        Particle.publish("vCell", String(fuel.getVCell()), 3600, PRIVATE);
        lastBatteryStateSentTimeStamp = now;
    }
}