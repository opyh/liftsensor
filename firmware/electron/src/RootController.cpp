#include "RootController.h"
#include "math.h"
#include "Diagnostics.h"
#include "TimeUtil.h"

bool locationRequested = false;


int locate(String args)
{
	locationRequested = true;
	return 1;
}


RootController::RootController(Battery &_battery, SensorDriver &_sensorDriver, NetworkDriver &_networkDriver)
:
	StateMachine(StateConnecting, "Core"),
	lastError(NULL),
	battery(_battery),
	sensorDriver(_sensorDriver),
	networkDriver(_networkDriver)
{

}


void RootController::init()
{
	Serial.println("\n\n\n\n\n[Core] Initializing magic... ✨ ");
	Serial.print("[Core] Hello, I am ");
	Serial.print(Particle.deviceID());
	Serial.println("!");
	Particle.function("locate", locate);
	sensorDriver.init();
	networkDriver.init();
	battery.init();
    Particle.variable("curTimeoutMs", intervalForInactivityEventMs);
}


void RootController::process()
{
	StateMachine::process();
	time_t now = millis();
	sensorDriver.process(now);
	networkDriver.process(now);
    battery.process(now);
}



void RootController::processState()
{
	if (locationRequested)
	{
		setState(StateLocating);
		locationRequested = false;
		return;
	}

	switch (state)
	{
		case StateConnecting:
			setStateAfter(StateSendingVersionDiagnostics, 500);
			break;
		case StateSendingVersionDiagnostics:
			setStateAfter(StateSendingCellularDiagnostics, 500);
			break;
		case StateSendingCellularDiagnostics:
			setStateAfter(StateWaitingForOvershot, 500);
			break;
		case StateInactive:
			if (sensorDriver.isOverSpeedThreshold()) setState(StateMeasuringPaused);
			// Exponential backoff
			setStateAfter(StateInactive, intervalForInactivityEventMs);
			break;
		case StateWaitingForOvershot:
			if (sensorDriver.isOverSpeedThreshold()) setState(StateMeasuringPaused);
			setStateAfter(StateInactive, IntervalForDetectingInactivityMs);
			break;
		case StateMeasuringPaused:
			setStateAfter(StateWaitingForOvershot, OvershotDetectionPauseIntervalMs);
			break;
		case StateError:
			setStateAfter(StateSendingCellularDiagnostics, intervalForErrorEventMs);
			break;
	}
}


void RootController::enterState()
{
	if (state != lastState && state != StateConnecting)
	{
		sendCurrentStateEvent();
	}

	switch (state)
	{
		case StateSendingVersionDiagnostics:
			Diagnostics::sendVersionDiagnostics();
			break;
		case StateSendingCellularDiagnostics:
			Diagnostics::sendCellularDiagnostics();
			Diagnostics::sendNetworkRegistrationStatus();
			break;
		case StateLocating:
			break;
		case StateWaitingForOvershot:
			break;
		case StateMeasuringPaused:
			sendOvershootEvent();
		    intervalForInactivityEventMs = FirstIntervalForInactivityEventMs;
			break;
		case StateInactive:
		    networkDriver.enqueueEvent("inactiveS", String(int(0.001 * (millis() - intervalForInactivityEventMs))));
	        intervalForInactivityEventMs *= ExponentialBackoffFactorForInactivityEvent;
			Serial.print("[Core] Increased interval for inactivity events to ");
			printTime(Serial, intervalForInactivityEventMs);
			Serial.println();
			break;
		case StateError:
			intervalForErrorEventMs *= 2;
			if (lastError)
			{
				Serial.print("[Core] Last error: ");
				Serial.print(lastError);
			}
			else
			{
				Serial.print("[Core] Unknown error.");
			}
			break;
	}
}


void RootController::exitState()
{
	switch (state)
	{
		case StateConnecting: break;
		case StateSendingVersionDiagnostics: break;
    	case StateSendingCellularDiagnostics: break;
		case StateLocating: break;
		case StateWaitingForOvershot: break;
		case StateMeasuringPaused: break;
		case StateInactive: break;
		case StateError: lastError = NULL; break;
	}
}


const char *RootController::nameForState(RootControllerState state)
{
	switch (state)
	{
		case StateConnecting: return "connecting to server";
		case StateSendingVersionDiagnostics: return "sending version";
		case StateSendingCellularDiagnostics: return "sending cellular info";
		case StateLocating: return "locating";
		case StateWaitingForOvershot: return "waiting for overshot";
		case StateMeasuringPaused: return "measuring paused";
		case StateInactive: return "inactive";
		case StateError: return "error";
	}

	return "Unknown state";
}


void RootController::sendCurrentStateEvent()
{
	const char *stateString = nameForState(state);
	Particle.variable("state", stateString, STRING);
	Particle.publish("diag/lastState", stateString, 3600, PRIVATE);
}


void RootController::sendOvershootEvent() {
	Serial.print("\nSpeed threshold exceeded, sending event with speed ");
	const auto speed = sensorDriver.getDCFilteredSpeed();
	Serial.println(speed, DEC);
	Serial.println();
	networkDriver.enqueueEvent("speed", String(speed));
}

