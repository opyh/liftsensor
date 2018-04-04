#ifndef ROOT_CONTROLLER_H
#define ROOT_CONTROLLER_H

#include "Controller.h"
#include "StateMachine.h"
#include "Battery.h"
#include "SensorDriver.h"
#include "NetworkDriver.h"

typedef enum {
	StateConnecting,
	StateSendingVersionDiagnostics,
	StateSendingCellularDiagnostics,
	StateWaitingForOvershot,
	StateMeasuringPaused,
	StateInactive,
	StateLocating,
	StateError,
} RootControllerState;

class RootController : public Controller, public StateMachine<RootControllerState>
{
public:
	RootController(Battery &_battery, SensorDriver &_sensorDriver, NetworkDriver &_networkDriver);

	void init();
	void process();

	void processState();
	void enterState();
	void exitState();
	const char *nameForState(RootControllerState state);

	void sendCurrentStateEvent();

private:
	const char *lastError;

	const time_t IntervalForDetectingInactivityMs = 10000;
	const time_t OvershotDetectionPauseIntervalMs = 10000;
	// This is for an exponentially increasing timeout for sending an event that signals that there
	// was no ride for a while
	const time_t FirstIntervalForInactivityEventMs = 30000;
	const int ExponentialBackoffFactorForInactivityEvent = 2;
	time_t intervalForInactivityEventMs = FirstIntervalForInactivityEventMs;
	time_t intervalForErrorEventMs = 15000;

	Battery &battery;
	SensorDriver &sensorDriver;
	NetworkDriver &networkDriver;

	void processMeasurements(time_t now);
	void sendOvershootEvent();
};

#endif
