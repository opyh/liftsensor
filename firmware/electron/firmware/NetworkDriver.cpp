#include "Particle.h"
#include <math.h>

#include "NetworkDriver.h"


const system_tick_t NetworkDriver::DefaultTTL = 10 * 60 * 1000;

NetworkDriver::NetworkDriver() :
	StateMachine(StateOffline, "Net"),
    publishInterval(500, &NetworkDriver::processEventQueue, *this, false)
{
}


void NetworkDriver::init() {

}


void NetworkDriver::process(time_t now) {
	StateMachine::process();
}


void NetworkDriver::processState() {
	switch (state)
	{
		case StateOffline:
            if (Particle.connected()) {
                setState(StateOnline);
            }
			break;
		case StateOnline:
			if (!Particle.connected()) {
                setState(StateOffline);
            }
			break;
	}
}


void NetworkDriver::enterState() {
	switch (state)
	{
		case StateOffline:
            if (publishInterval.isActive()) {
                publishInterval.stop();
            }
			break;
		case StateOnline:
			if (!Particle.connected()) {
                setState(StateOffline);
            }
            if (!publishInterval.isActive()) {
                publishInterval.start();
            }
			break;
	}
}


void NetworkDriver::exitState() {

}


const char *NetworkDriver::nameForState(NetworkDriverState state) {
    switch(state) {
        case StateOffline: return "offline";
    	case StateOnline: return "online";
    }
    return "Unknown state";
}


void NetworkDriver::processEventQueue() {
    if (eventQueue.empty()) return;
    NetworkDriverEvent event = eventQueue.front();
    if (millis() - event.timestamp > event.ttl) {
        eventQueue.pop();
        return;
    }
    if (Particle.publish(event.name, event.data, 3600, PRIVATE)) {
        eventQueue.pop();
        Serial.printlnf("\r\n[Net] published event '%s' (data: %s).", event.name.c_str(), event.data.c_str());
    }
};


void NetworkDriver::enqueueEvent(String name, String data, system_tick_t ttl) {
    NetworkDriverEvent event = { .name = name, .data = data, .timestamp = millis(), ttl: ttl };
    if (eventQueue.size() > MaxQueueSize) {
        eventQueue.pop();
    }
    eventQueue.push(event);
    Serial.printlnf("\r\n[Net] Enqueued event '%s' (data: %s).", name.c_str(), data.c_str());
}
