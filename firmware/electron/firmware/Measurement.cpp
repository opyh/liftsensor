#include "Particle.h"
#include <math.h>

#include "Measurement.h"

namespace Measurement {
    const double analogResolution = 4096.0;
    const double supplyVoltage = 3.3;
    const double sensorMaximumInG = 3.0;
    const double voltsPerG = 0.5 * (supplyVoltage / sensorMaximumInG);
    const double gravityOffsetInG = 1.0;

    double axisValue[NUMBER_OF_AXES];
    double averagedAxisValue[NUMBER_OF_AXES];
    double exponentialAverageAlpha = 0.1;

    double thresholdInG = 0.20;

    double accelerationInG = 0.0;
    double maxOvershootValueSinceSendingG = 0.0;


    int setThresholdInG(String newTime) {
        thresholdInG = newTime.toFloat();
        return 0;
    }


    double accelerationFromPinValue(int pinValue) {
        double volts = -0.5 * supplyVoltage + (supplyVoltage * pinValue / analogResolution);
        return volts / voltsPerG;
    }

    double getAccelerationInG() {
        return accelerationInG;
    }

    double getMaxOvershootValueSinceSendingG() {
        return maxOvershootValueSinceSendingG;
    }


    bool isOvershotSinceSending() {
        return fabs(accelerationInG) > fabs(maxOvershootValueSinceSendingG);
    }


    bool isOverThreshold() {
        return fabs(accelerationInG) > thresholdInG;
    }


    void resetMaxOvershootValueSinceSending() {
        maxOvershootValueSinceSendingG = 0;
    }


    bool updateOvershootMeasurement(time_t now) {
        if (isOverThreshold() && isOvershotSinceSending()) {
            maxOvershootValueSinceSendingG = accelerationInG;
            return true;
        }
        return false;
    }


    void setup() {
        setADCSampleTime(ADC_SampleTime_480Cycles);

        Particle.variable("voltsPerG", voltsPerG);
        Particle.variable("gravOffsetG", gravityOffsetInG);
        Particle.variable("x", axisValue[0]);
        Particle.variable("y", axisValue[1]);
        Particle.variable("z", axisValue[2]);
        Particle.variable("g", accelerationInG);
        Particle.variable("maxG", maxOvershootValueSinceSendingG);
        Particle.function("setThreshInG", setThresholdInG);
    }


    void measure() {
        // axisValue[0] = accelerationMeasuredOnPin(A0);
        // axisValue[1] = accelerationMeasuredOnPin(A1);
        // axisValue[2] = accelerationMeasuredOnPin(A2);

        // the real z pin would be on A2, but on the hardware I tested this code with, z line is broken
        // so we use A0 = x axis for debugging instead
        axisValue[2] = accelerationFromPinValue(analogRead(A2));
        averagedAxisValue[2] =        exponentialAverageAlpha  * axisValue[2] +
                            (1.0 - exponentialAverageAlpha) * averagedAxisValue[2];
        Serial.print(analogRead(A0), DEC);
        Serial.print(" ");
        Serial.print(axisValue[2] - gravityOffsetInG, DEC);
        Serial.print(" ");
        Serial.println(averagedAxisValue[2] - gravityOffsetInG, DEC);
        accelerationInG = averagedAxisValue[2] - gravityOffsetInG;
    }
}