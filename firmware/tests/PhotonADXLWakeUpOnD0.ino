#include "Particle.h"

SYSTEM_MODE(SEMI_AUTOMATIC);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

void setup() {
    switch(System.resetReason()) {
        case RESET_REASON_PANIC:
            System.enterSafeMode();
            break;
        default:
            break;
    }
    if (System.resetReason() == RESET_REASON_PANIC) {
       System.enterSafeMode();
   }
}

long movement = 0;

void loop() {
	if(!Particle.connected()) {
	    Particle.connect();
	}
	Particle.publish("movement", String(movement++));
    delay(5000);
	Particle.disconnect();
	System.sleep(D0, CHANGE, 3600);
}
