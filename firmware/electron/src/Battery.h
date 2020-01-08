#ifndef BATTERY_H
#define BATTERY_H

#include "Particle.h"
#include "Driver.h"

class Battery : Driver {
    FuelGauge fuel;
    double stateOfCharge = 0.0;
    time_t lastBatteryStateSentTimeStamp = 0;
    const time_t batteryTimeInterval = 10 * 60 * 1000;

public:
    void init();
    void process(time_t now);
};

#endif