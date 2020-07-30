#include "RootController.h"
#include "math.h"
#include "Diagnostics.h"
#include "TimeUtil.h"
#include "Settings.h"

bool locationRequested = false;


int locate(String args)
{
	locationRequested = true;
	return 1;
}

int sendNetworkRegistrationStatus(String args) {
	Diagnostics::sendNetworkRegistrationStatus();
	return 0;
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
	delay(3000);
	Serial.println("\r\n\n\n\n\n[Core] Initializing magic... ✨ ");
	Serial.print("[Core] Hello, I am ");
	Serial.print(Particle.deviceID());
	Serial.println("!");
	Particle.function("locate", locate);
	Particle.function("net", sendNetworkRegistrationStatus);
	Settings::setupEEPROMFunctions();
	sensorDriver.init();
	networkDriver.init();
	battery.init();
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
			if (sensorDriver.isOverSpeedThreshold()) {
				setState(StateMeasuringPaused);
			}
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
		case StateSendingCellularDiagnostics:
			Diagnostics::sendCellularDiagnostics();
			break;
		case StateLocating:
			// startLocating();
			break;
		case StateWaitingForOvershot:
			sensorDriver.setupFirstAverageValue();
			break;
		case StateMeasuringPaused:
			sendOvershootEvent();
		    intervalForInactivityEventMs = FirstIntervalForInactivityEventMs;
			break;
		case StateInactive: {
			const auto &settings = Settings::getFromEEPROM();
			String data = String::format(
				"{\"v\":0,\"s\":\"%s\",\"e\":\"%s\",\"t\":\"%s\",\"d\":%d}",
				settings.sourceId,
				settings.equipmentInfoId,
				settings.token,
				lastActivityMs ? int(0.001 * (millis() - lastActivityMs)) : -1
			);
			networkDriver.enqueueEvent("speed", data);


	        intervalForInactivityEventMs *= ExponentialBackoffFactorForInactivityEvent;
			Serial.print("\r\n[Core] Increased interval for inactivity events to ");
			printTime(Serial, intervalForInactivityEventMs);
			Serial.println();
		    Particle.variable("timeoutMs", intervalForInactivityEventMs);

			break;
		}
		case StateError:
			intervalForErrorEventMs *= 2;
			if (lastError)
			{
				Serial.print("\r\n[Core] Last error: ");
				Serial.print(lastError);
			}
			else
			{
				Serial.print("\r\n[Core] Unknown error.");
			}
			break;
	}
}


void RootController::exitState()
{
	switch (state)
	{
		case StateConnecting: break;
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
	lastActivityMs = millis();
	const auto speed = sensorDriver.getDCFilteredSpeed();
	Serial.printlnf("\r\nSpeed threshold exceeded at %d, sending event with speed %0.4f", lastActivityMs, speed);

	const auto &settings = Settings::getFromEEPROM();
	String data = String::format(
		"{\"v\":%0.4f,\"s\":\"%s\",\"e\":\"%s\",\"t\":\"%s\",\"w\":true,\"d\":0}",
		speed,
		settings.sourceId,
		settings.equipmentInfoId,
		settings.token
	);
	networkDriver.enqueueEvent("speed", data);
}

