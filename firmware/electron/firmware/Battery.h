#ifndef BATTERY_H
#define BATTERY_H

#include "Particle.h"

void setupBatteryMonitoring();
void processBatteryStatusEvents(time_t now);

#endif