#ifndef NETWORK_DRIVER_H
#define NETWORK_DRIVER_H

#include "Particle.h"
#include "StateMachine.h"
#include "Driver.h"
#include <queue>

typedef enum {
    StateOffline,
	StateOnline,
} NetworkDriverState;

struct NetworkDriverEvent {
    String name;
    String data;
    system_tick_t timestamp;
    system_tick_t ttl;
};

class NetworkDriver : StateMachine<NetworkDriverState>, Driver {
public:
    NetworkDriver();
    void init();
    void process(time_t now);

	void processState();
	void enterState();
	void exitState();
	const char *nameForState(NetworkDriverState state);

    void processEventQueue();
    void enqueueEvent(String name, String data, system_tick_t ttl = 10 * 60 * 1000);
    std::queue<NetworkDriverEvent> eventQueue;
    Timer publishInterval;

    static const size_t MaxQueueSize = 10;
};

#endif