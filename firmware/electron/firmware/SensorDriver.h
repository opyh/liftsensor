#ifndef MEASUREMENT_CONTROLLER_H
#define MEASUREMENT_CONTROLLER_H

#define NUMBER_OF_AXES 3

#include "Particle.h"
#include "StateMachine.h"
#include "Driver.h"

typedef enum {
    StateCalibrating,
    StateElevatorStopped,
	StateElevatorGoingUp,
    StateElevatorGoingDown,
} SensorDriverState;

class SensorDriver : StateMachine<SensorDriverState>, Driver {
public:
    // Values from data sheet:
    // http://www.analog.com/media/en/technical-documentation/data-sheets/ADXL335.pdf
    const double SupplyVoltage = 3.3;
    const double SensorMaximumInG = 3;
    const double VoltsPerG = 0.300;
    const double GravityOffsetInG = 1.0;
    const double AxisBiasVoltages[NUMBER_OF_AXES] = { 1.6f, 1.6f, 1.67f };
    const double ExponentialAverageAlpha = 0.3;
    const double ThresholdInG = 0.10;

    // https://docs.particle.io/reference/firmware/electron/#analogread-adc-
    const double AnalogResolution = 4096.0;

    int pinValues[NUMBER_OF_AXES];
    double axisValues[NUMBER_OF_AXES];
    double averagedAxisValues[NUMBER_OF_AXES];

    double accelerationInG = 0.0;

    time_t lastOvershootTime = 0;

    SensorDriver();
    void init();
    void process(time_t now);
	void processState();
	void enterState();
	void exitState();
	const char *nameForState(SensorDriverState state);

    bool isOverThreshold();
    double getAccelerationInG();

    void measure();
    void measureOneAxis(size_t index, int pin);
    void setupParticleCloud();
    void setupFirstAverageValue();
    double voltageFromPinValue(int pinValue);
    double accelerationFromPinValue(size_t axisIndex, int pinValue);
};

#endif