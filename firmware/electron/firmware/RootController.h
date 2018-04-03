#ifndef ROOT_CONTROLLER_H
#define ROOT_CONTROLLER_H

#include "Controller.h"
#include "StateMachine.h"
#include "Battery.h"
#include "SensorDriver.h"

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
	RootController(Battery &_battery, SensorDriver &_sensorDriver);

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
	const time_t FirstIntervalForInactivityEventMs = 10000;
	time_t intervalForInactivityEventMs = 10000;
	time_t intervalForErrorEventMs = 15000;

	time_t lastSentTimeStamp;
	time_t lastOvershootTime;

	Battery &battery;
	SensorDriver &sensorDriver;

	void processMeasurements(time_t now);
	void sendOvershootEvent(time_t now);
	void reactToOvershootEvents();
};

#endif
