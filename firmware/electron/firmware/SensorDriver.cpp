#include "Particle.h"
#include <math.h>

#include "SensorDriver.h"

// int setThresholdInG(String newTime) {
//     thresholdInG = newTime.toFloat();
//     return 0;
// }


SensorDriver::SensorDriver() {
}


double SensorDriver::voltageFromPinValue(int pinValue) {
    return (SupplyVoltage * pinValue / AnalogResolution);
}


double SensorDriver::accelerationFromPinValue(size_t axisIndex, int pinValue) {
    double volts = voltageFromPinValue(pinValue) - AxisBiasVoltages[axisIndex];
    return volts / VoltsPerG;
}


double SensorDriver::getDCFilteredSpeed() {
    return smoothenedFilteredAccumulatedSpeeds[0];
}

bool SensorDriver::isOverSpeedThreshold() {
    return fabs((float) getDCFilteredSpeed()) > SpeedThreshold;
}

void SensorDriver::updateMaxSpeed() {
    auto speed = getDCFilteredSpeed();
    if (fabs((float) speed) > fabs((float) maxSpeed)) {
        maxSpeed = speed;
    }
}


void SensorDriver::setCloudVariables() {
    Particle.variable("x", smoothenedAccelerations[0]);
    Particle.variable("y", smoothenedAccelerations[1]);
    Particle.variable("z", smoothenedAccelerations[2]);
    Particle.variable("speed", smoothenedFilteredAccumulatedSpeeds[2]);
    Particle.variable("maxSpeed", maxSpeed);
}


void SensorDriver::setupFirstAverageValue() {
    measure();
    for (size_t i = 0; i < NUMBER_OF_AXES; i++) {
        smoothenedAccelerations[i] = accelerations[i];
        dcFilteredSpeeds[i] = 0;
        smoothenedDCFilteredSpeeds[i] = 0;
        dcFilteredAveragedDCFilteredSpeeds[i] = 0;
        smoothenedFilteredAccumulatedSpeeds[i] = 0;
    }
}


void SensorDriver::init() {
    // Longer sample time for smoother values
    setADCSampleTime(ADC_SampleTime_480Cycles);
    setupFirstAverageValue();
}


void SensorDriver::measureOneAxis(size_t index, int pin) {
    pinValues[index] = analogRead(pin);
    accelerations[index] = accelerationFromPinValue(index, pinValues[index]);
    smoothenedAccelerations[index] =
        ExponentialAverageAlpha * accelerations[index] +
        (1.0 - ExponentialAverageAlpha) * smoothenedAccelerations[index];

    // 1. Accumulating past acceleration values to calculate speed. Filter out gravity first using
    // a DC bias filter

    dcFilteredSpeeds[index] += accelerations[index] - smoothenedAccelerations[index];

    // 2. average signal to remove high-frequency noise. Without this, a sudden movement like a
    // train passing nearby or an entering passenger could cause an overshoot event.

    smoothenedDCFilteredSpeeds[index] =
        ExponentialAverageAlpha * dcFilteredSpeeds[index] +
        (1.0 - ExponentialAverageAlpha) * smoothenedDCFilteredSpeeds[index];

    // 3. The signal still has a DC bias. Remove it.

    dcFilteredAveragedDCFilteredSpeeds[index] = dcFilteredSpeeds[index] - smoothenedDCFilteredSpeeds[index];

    // 4. Another low-pass filter to remove jitter.

    smoothenedFilteredAccumulatedSpeeds[index] =
            ExponentialAverageAlpha * dcFilteredAveragedDCFilteredSpeeds[index] +
        (1.0 - ExponentialAverageAlpha) * smoothenedFilteredAccumulatedSpeeds[index];

}


void SensorDriver::measure() {
    measureOneAxis(0, A0);
    measureOneAxis(1, A1);
    measureOneAxis(2, A2);
    updateMaxSpeed();
}


void SensorDriver::process(time_t now) {
    measure();
    setCloudVariables();
    printStatus();
}

void SensorDriver::printStatus() {
    // Debouncing
    auto now = millis();
    if (now - lastPrintTime < 200) return;
    lastPrintTime = now;

    Serial.printf("\r");
    for (size_t i = 0; i < NUMBER_OF_AXES; i++) {
        Serial.printf(
            "%c %.2fV %+.2fg %+.2fg %+.2fg  │  ",
            'x' + i,
            voltageFromPinValue(pinValues[i]),
            accelerations[i],
            smoothenedAccelerations[i],
            accelerations[i] - smoothenedAccelerations[i]
        );
    }

    Serial.printf(
        " -> v = %+.2f / %+.2f",
        dcFilteredAveragedDCFilteredSpeeds[2],
        smoothenedFilteredAccumulatedSpeeds[2]
    );
}
