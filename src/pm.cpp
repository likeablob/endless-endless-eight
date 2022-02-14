#include "pm.h"

namespace PM {

void enableBusPower() {
    pinMode(PIN_BUS_POWER_SW, OUTPUT);
    digitalWrite(PIN_BUS_POWER_SW, HIGH);
}

void disableBusPower() {
    pinMode(PIN_BUS_POWER_SW, OUTPUT);
    digitalWrite(PIN_BUS_POWER_SW, LOW);
}

bool isPeriodicTaskRequested(uint32_t status) {
    return status & PM_STATUS_PERIODIC_TASK_REQ;
}

void clearPeriodicTaskRequest(uint32_t &status) {
    status &= ~PM_STATUS_PERIODIC_TASK_REQ;
}

bool isWakeVoltageSatisfied(uint32_t status) {
    return status & PM_STATUS_WAKE_VOLTAGE_SATISFIED;
}

bool isEmergencyTaskRequested(uint32_t status) {
    return status & PM_STATUS_EMERGENCY_TASK_REQ;
}
void clearEmergencyTaskRequest(uint32_t &status) {
    status &= ~PM_STATUS_EMERGENCY_TASK_REQ;
};

} // namespace PM
