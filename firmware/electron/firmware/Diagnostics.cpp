#include "Diagnostics.h"
#include "Particle.h"

namespace Diagnostics {

    int networkRegistrationStatusCallback(int type, const char *buf, int len, char *status) {

        if ((type == TYPE_PLUS) && status)
        {
            // Serial.printlnf("Network registration status = %s\r\n", buf);
            if (sscanf(buf, "\r\n+COPS: %[^\r]\r\n", status) == 1) {}
        }
        return WAIT;
    }

    void sendNetworkRegistrationStatus() {
        Serial.printlnf("\r\nChecking network registration status...");
        // AT+COPS? +COPS: 0,0,"vodafone IT",2
        char status[32] = "";
        if ((RESP_OK == Cellular.command(networkRegistrationStatusCallback, status, 20000, "AT+COPS?\r\n")) && (strcmp(status, "") != 0))
        {
            Particle.publish("diag/net", status, 3600, PRIVATE);
        }
        else
        {
            Serial.println("\r\nCould not get network registration status.");
        }
    }

    void sendCellularDiagnostics() {
        CellularSignal sig = Cellular.RSSI();

        // -127 to -1 as int.
        // The signal strength with range -113dBm to -51dBm (in 2dBm steps).
        // This variable also doubles as an error response for the entire struct;
        // Positive return values indicate an error with 1 indicating a
        // Cellular chip error / time-out and 2 indicating that the rssi
        // value is not known, not detectable or currently not available
        char rssiString[5]; // one sign + three chars + one terminating \0
        snprintf(rssiString, 5, "%d", sig.rssi);
        Particle.publish("diag/rssi", rssiString, 3600, PRIVATE);

        // 0 to 49 as int.
        // number in UMTS RAT indicating the Energy per Chip/Noise ratio
        // in dB levels of the current cell. This value ranges from 0 to
        // 49, higher numbers indicate higher signal quality.
        char qualString[5]; // one sign + three chars + one terminating \0
        snprintf(qualString, 5, "%d", sig.qual);
        Particle.publish("diag/qual", qualString, 3600, PRIVATE);
    }
}
