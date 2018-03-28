#include "Battery.h"

FuelGauge fuel;
double stateOfCharge = 0.0;
time_t lastBatteryStateSentTimeStamp = 0;
const time_t batteryTimeInterval = 10 * 60 * 1000;

void setupBatteryMonitoring() {
    Particle.variable("charge", stateOfCharge);
}

void processBatteryStatusEvents(time_t now) {
    if ((now - lastBatteryStateSentTimeStamp) > batteryTimeInterval) {
        stateOfCharge = fuel.getSoC();
        Particle.publish("charge", String(fuel.getSoC()), 3600, PRIVATE);
        Particle.publish("vCell", String(fuel.getVCell()), 3600, PRIVATE);
        lastBatteryStateSentTimeStamp = now;
    }
}