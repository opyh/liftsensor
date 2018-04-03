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


RootController::RootController(Battery &_battery, SensorDriver &_sensorDriver)
:
	StateMachine(StateConnecting, "Core"),
	lastError(NULL),
	battery(_battery),
	sensorDriver(_sensorDriver)
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
	battery.init();
    Particle.variable("curTimeoutMs", intervalForInactivityEventMs);
}


void RootController::process()
{
	StateMachine::process();
	time_t now = millis();
	sensorDriver.process(now);
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
			// Exponential backoff
			setStateAfter(StateSendingCellularDiagnostics, intervalForInactivityEventMs);
			reactToOvershootEvents();
			break;
		case StateWaitingForOvershot:
			reactToOvershootEvents();
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


void RootController::reactToOvershootEvents() {
	if (sensorDriver.isOverThreshold()) {
		lastOvershootTime = millis();
		sendOvershootEvent(lastOvershootTime);
		setState(StateMeasuringPaused);
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
		    intervalForInactivityEventMs = FirstIntervalForInactivityEventMs;
			break;
		case StateInactive:
		    Particle.publish("noAccel", "", 3600, PRIVATE);
	        intervalForInactivityEventMs *= 2;
			Serial.print("[Core] Doubled interval for inactivity events to ");
			printTime(Serial, intervalForInactivityEventMs);
			Serial.println("s");
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
	Particle.publish("diag/lastState", stateString, 120, PRIVATE);
}


void RootController::sendOvershootEvent(time_t now) {
	Serial.print("\n\nSending maximal acceleration since last sent maxAccel event: ");
	const auto accelerationInG = sensorDriver.getAccelerationInG();
	Serial.println(accelerationInG, DEC);
	Serial.println();
	Particle.publish("accelR", String(accelerationInG), 3600, PRIVATE);
	lastSentTimeStamp = now;
}

