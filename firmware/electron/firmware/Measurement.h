#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#define NUMBER_OF_AXES 3

#include "Particle.h"

namespace Measurement {
    void setup();
    void measure();
    bool isOverThreshold();
    bool isOvershotSinceSending();
    // returns true if overshot since last sending
    bool updateOvershootMeasurement(time_t now);
    void resetMaxOvershootValueSinceSending();
    double getAccelerationInG();
    double getMaxOvershootValueSinceSendingG();
}

#endif