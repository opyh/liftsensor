#include "Particle.h"
#include <math.h>

#include "SensorDriver.h"

// int setThresholdInG(String newTime) {
//     thresholdInG = newTime.toFloat();
//     return 0;
// }


SensorDriver::SensorDriver() :
	StateMachine(StateElevatorStopped, "Sensor")
{
}


double SensorDriver::voltageFromPinValue(int pinValue) {
    return (SupplyVoltage * pinValue / AnalogResolution);
}


double SensorDriver::accelerationFromPinValue(size_t axisIndex, int pinValue) {
    double volts = voltageFromPinValue(pinValue) - AxisBiasVoltages[axisIndex];
    return volts / VoltsPerG;
}


double SensorDriver::getAccelerationInG() {
    return accelerationInG;
}


bool SensorDriver::isOverThreshold() {
    return fabs((float) accelerationInG) > ThresholdInG;
}


void SensorDriver::setupParticleCloud() {
    Particle.variable("voltsPerG", VoltsPerG);
    Particle.variable("gravOffsetG", GravityOffsetInG);
    Particle.variable("x", averagedAxisValues[0]);
    Particle.variable("y", averagedAxisValues[1]);
    Particle.variable("z", averagedAxisValues[2]);
    Particle.variable("g", accelerationInG);
    Particle.variable("lastMovedAt", lastOvershootTime);
}


void SensorDriver::setupFirstAverageValue() {
    measure();
    for (size_t i = 0; i < NUMBER_OF_AXES; i++) {
        averagedAxisValues[i] = axisValues[i];
    }
}


void SensorDriver::init() {
    // Longer sample time for smoother values
    setADCSampleTime(ADC_SampleTime_480Cycles);
    setupParticleCloud();
    setupFirstAverageValue();
}


void SensorDriver::measureOneAxis(size_t index, int pin) {
    pinValues[index] = analogRead(pin);
    axisValues[index] = accelerationFromPinValue(index, pinValues[index]);
    averagedAxisValues[index] =        ExponentialAverageAlpha  * axisValues[index] +
                        (1.0 - ExponentialAverageAlpha) * averagedAxisValues[index];
}


void SensorDriver::measure() {
    measureOneAxis(0, A0);
    measureOneAxis(1, A1);
    measureOneAxis(2, A2);
}


void SensorDriver::process(time_t now) {
    measure();

    // For the first prototype PCB, the real z pin would be on A2, but the z line is broken.
    // so we use A0 = x axis for debugging instead. Turn the device on the side for this.
    // axisValues[2] = accelerationFromPinValue(analogRead(A0));
    // int axisIndex = 2;
    // accelerationInG = averagedAxisValues[axisIndex] - GravityOffsetInG;

    // Simple DC filtering to remove earth gravity bias
    double x = axisValues[0] - averagedAxisValues[0];
    double y = axisValues[1] - averagedAxisValues[1];
    double z = axisValues[2] - averagedAxisValues[2];

    accelerationInG = z;

    // Alternative: Calculate length of the 3D vector to use all axes.
    // This is a bit more error prone and might cause overshoots when people enter/leave the
    // elevator:
    // accelerationInG = sqrt(
    //     pow(x, 2.0) +
    //     pow(y, 2.0) +
    //     pow(z, 2.0)
    // );

    for (size_t i = 0; i < NUMBER_OF_AXES; i++) {
        Serial.printf(
            "%c %.2fV %+.2fg %+.2fg %+.2fg  │  ",
            'x' + i,
            voltageFromPinValue(pinValues[i]),
            axisValues[i],
            averagedAxisValues[i],
            axisValues[i] - averagedAxisValues[i]
        );
    }

    Serial.printlnf(
        "  filtered %+.2fg",
        accelerationInG
    );
}


void SensorDriver::processState() {

}


void SensorDriver::enterState() {

}


void SensorDriver::exitState() {

}


const char *SensorDriver::nameForState(SensorDriverState state) {
    switch(state) {
        case StateCalibrating: return "calibrating";
	    case StateElevatorStopped: return "elevator stopped";
	    case StateElevatorGoingUp: return "elevator going up";
        case StateElevatorGoingDown: return "elevator going down";
    }
    return "Unknown state";
}
