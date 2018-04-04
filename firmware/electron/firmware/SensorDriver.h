#ifndef MEASUREMENT_CONTROLLER_H
#define MEASUREMENT_CONTROLLER_H

#define NUMBER_OF_AXES 3

#include "Particle.h"
#include "Driver.h"


class SensorDriver : Driver {
public:
    SensorDriver();

    void init();
    void process(time_t now);

    bool isOverSpeedThreshold();
    double getDCFilteredSpeed();

private:
    // Values from data sheet:
    // http://www.analog.com/media/en/technical-documentation/data-sheets/ADXL335.pdf
    const double SupplyVoltage = 3.3;
    const double SensorMaximumInG = 3;
    const double VoltsPerG = 0.300;
    const double GravityOffsetInG = 1.0;
    const double AxisBiasVoltages[NUMBER_OF_AXES] = { 1.6f, 1.6f, 1.67f };
    const double ExponentialAverageAlpha = 0.02;
    const double SpeedThreshold = 0.4;

    // https://docs.particle.io/reference/firmware/electron/#analogread-adc-
    const double AnalogResolution = 4096.0;

    int pinValues[NUMBER_OF_AXES];
    double accelerations[NUMBER_OF_AXES] = { 0, 0, 1 };
    double smoothenedAccelerations[NUMBER_OF_AXES] = { 0, 0, 1 };
	double dcFilteredSpeeds[NUMBER_OF_AXES] = { 0, 0, 0 };
    double smoothenedDCFilteredSpeeds[NUMBER_OF_AXES] = { 0, 0, 0 };
    double dcFilteredAveragedDCFilteredSpeeds[NUMBER_OF_AXES] = { 0, 0, 0 };
    double smoothenedFilteredAccumulatedSpeeds[NUMBER_OF_AXES] = { 0, 0, 0 };

    double accelerationInG = 0.0;
    double smoothenedAccelerationInG = 0.0;

    time_t lastPrintTime = 0;

    void measure();
    void measureOneAxis(size_t index, int pin);
    void setupParticleCloud();
    void setupFirstAverageValue();
    double voltageFromPinValue(int pinValue);
    double accelerationFromPinValue(size_t axisIndex, int pinValue);
    void printStatus();
};

#endif