#include <math.h>
FuelGauge fuel;

time_t lastSentTimeStamp = 0;
time_t lastSentSerialTimeStamp = 0;
time_t lastBatteryStateSentTimeStamp = 0;
time_t batteryTimeInterval = 10 * 60 * 1000;
time_t debounceTimeInMilliseconds = 10000;
time_t serialDebounceTimeInMilliseconds = 100;
time_t mainLoopDelay = 100;
time_t lastOvershootTime = 0;
time_t startTimeoutForZeroEventInMilliseconds = 10000;
time_t timeoutForZeroEventInMilliseconds = 10000;


double analogResolution = 4096.0;
double supplyVoltage = 3.3;
double stateOfCharge = 0.0;
double maximalMeasuredAbsoluteAccelerationInG = 5.0;
double voltsPerG = supplyVoltage / (maximalMeasuredAbsoluteAccelerationInG * 2.0);
double gravityOffsetInG = 1.0;

double x = 0.0;
double y = 0.0;
double z = 0.0;

double xValueForOneG = 0.0;
double yValueForOneG = 0.0;
double zValueForOneG = 0.0;

double thresholdInG = 0.10;
double absoluteAccelerationInG = 0.0;
double maxAbsoluteOvershootValueSinceSendingG = 0.0;
double maxOvershootValueSinceSendingG = 0.0;

int setDebounceTimeInMilliseconds(String newTime) {
    debounceTimeInMilliseconds = newTime.toInt();
    return 0;
}

void setup() {
    Serial.begin(115200);

    Particle.variable("voltsPerG", voltsPerG);
    Particle.variable("gravOffsetG", gravityOffsetInG);
    Particle.variable("x", x);
    Particle.variable("y", y);
    Particle.variable("z", z);
    Particle.variable("loopDelay", mainLoopDelay);
    Particle.variable("publishDelay", debounceTimeInMilliseconds);
    Particle.variable("lastMovedAt", lastOvershootTime);
    Particle.variable("serialDelay", serialDebounceTimeInMilliseconds);
    Particle.variable("accel", absoluteAccelerationInG);
    Particle.variable("maxAccel", maxOvershootValueSinceSendingG);
    Particle.variable("zeroTimeout", timeoutForZeroEventInMilliseconds);
    Particle.variable("charge", stateOfCharge);
    Particle.function("setPubDelay", setDebounceTimeInMilliseconds);
}

double accelerationMeasuredOnPin(int pin) {
    double volts = -0.5 * supplyVoltage + (supplyVoltage * (double) analogRead(pin) / analogResolution);
    return volts / voltsPerG;
}

void loop() {
    delay(mainLoopDelay);

    time_t now = millis();

    x = accelerationMeasuredOnPin(A0);
    y = accelerationMeasuredOnPin(A1);
    z = accelerationMeasuredOnPin(A2);
    
    absoluteAccelerationInG = fabs(z - gravityOffsetInG);
    double accelerationInG = z - gravityOffsetInG;

    bool isReadyForNextEvent = (now - lastSentTimeStamp) > debounceTimeInMilliseconds;
    bool isOverThreshold = absoluteAccelerationInG > thresholdInG;
    bool isOvershot = false;

    if (isOverThreshold && absoluteAccelerationInG > maxAbsoluteOvershootValueSinceSendingG) {
        lastOvershootTime = millis();
        maxAbsoluteOvershootValueSinceSendingG = absoluteAccelerationInG;
        maxOvershootValueSinceSendingG = accelerationInG;
        isOvershot = true;
        timeoutForZeroEventInMilliseconds = startTimeoutForZeroEventInMilliseconds;
    }

    bool hasOvershotSinceSending = lastOvershootTime > lastSentTimeStamp;
    bool hasTimedOut = (now - lastSentTimeStamp) > timeoutForZeroEventInMilliseconds;

    if (hasTimedOut) {
        Particle.publish("noAccel", String((now - lastSentTimeStamp) / 1000), 3600, PRIVATE);
        timeoutForZeroEventInMilliseconds *= 2;
    }

    if (isReadyForNextEvent && hasOvershotSinceSending) {
        Serial.print("Sending maximal acceleration since last sent maxAccel event");
        Serial.println(maxAbsoluteOvershootValueSinceSendingG);
        Particle.publish("maxAccelAbs", String(maxAbsoluteOvershootValueSinceSendingG), 3600, PRIVATE);
        Particle.publish("maxAccelR", String(maxOvershootValueSinceSendingG), 3600, PRIVATE);
        lastSentTimeStamp = now;
        maxAbsoluteOvershootValueSinceSendingG = 0.0;
    }

    bool isReadyForNextSerialEvent = (now - lastSentSerialTimeStamp) > serialDebounceTimeInMilliseconds;
    if (isOverThreshold && isReadyForNextSerialEvent) {
        if (isOvershot) {
            Serial.print("* ");
        }
        Serial.println(String(absoluteAccelerationInG));
    }

    if ((now - lastBatteryStateSentTimeStamp) > batteryTimeInterval) {
        stateOfCharge = fuel.getSoC();
        Particle.publish("charge", String(fuel.getSoC()), 3600, PRIVATE);
        Particle.publish("vCell", String(fuel.getVCell()), 3600, PRIVATE);
        lastBatteryStateSentTimeStamp = now;
    }
}