#include "Particle.h"
#include "math.h"

#include "Battery.h"
#include "Measurement.h"


const time_t serialDebounceTimeMs = 1000;
const time_t mainLoopDelay = 10;

time_t lastSentTimeStamp = 0;
time_t lastSentSerialTimeStamp = 0;
time_t lastOvershootTime = 0;
time_t debounceTimeMs = 10000;


int setDebounceTimeMs(String newTime) {
    debounceTimeMs = newTime.toInt();
    return 0;
}

// This is for an exponentially increasing timeout for sending an event that signals that there
// was no ride for a while
const time_t firstIntervalForInactivityEventMs = 10000;
time_t intervalForInactivityEventMs = 10000;


void sendOvershootEvent(time_t now) {
    bool isReadyForNextEvent = (now - lastSentTimeStamp) > debounceTimeMs;
    bool hasOvershootSinceSending = lastOvershootTime > lastSentTimeStamp;
    if (isReadyForNextEvent && hasOvershootSinceSending) {
        Serial.print("Sending maximal acceleration since last sent maxAccel event");
        const auto maxOvershoot = Measurement::getMaxOvershootValueSinceSendingG();
        Serial.println(maxOvershoot, DEC);
        Particle.publish("maxAccelR", String(maxOvershoot), 3600, PRIVATE);
        lastSentTimeStamp = now;
        Measurement::resetMaxOvershootValueSinceSending();
    }
}


void printStatus(time_t now) {
    bool isOvershot = Measurement::isOvershotSinceSending();
    bool isReadyForNextSerialEvent = (now - lastSentSerialTimeStamp) > serialDebounceTimeMs;
    if (!isOvershot && !isReadyForNextSerialEvent) return;
    Serial.print(Measurement::getAccelerationInG(), DEC);
    if (isOvershot) Serial.print(" *");
    Serial.println();
}


void processTimeoutEvents(time_t now) {
    bool hasTimedOut = (now - lastSentTimeStamp) > intervalForInactivityEventMs;
    if (hasTimedOut) {
        Particle.publish("noAccel", String((now - lastSentTimeStamp) / 1000), 3600, PRIVATE);
        intervalForInactivityEventMs *= 2;
    }
}


void setup() {
    Serial.begin(115200);

    Measurement::setup();
    setupBatteryMonitoring();

    Particle.variable("publishDelay", debounceTimeMs);
    Particle.function("setPubDelay", setDebounceTimeMs);
    Particle.variable("loopDelay", mainLoopDelay);

    Particle.variable("lastMovedAt", lastOvershootTime);

    Particle.variable("curTimeoutMs", intervalForInactivityEventMs);
}


void loop() {
    delay(mainLoopDelay);

    time_t now = millis();

    Measurement::measure();

    processTimeoutEvents(now);

    if (Measurement::updateOvershootMeasurement(now)) {
        lastOvershootTime = now;
        intervalForInactivityEventMs = firstIntervalForInactivityEventMs;
    }

    // if (Measurement::isOverThreshold()) {
    //     printStatus(now);
    // }

    processBatteryStatusEvents(now);
}
