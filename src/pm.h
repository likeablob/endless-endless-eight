#include "ulp_common.h"
#include <Arduino.h>
#include <stdint.h>

#define PIN_BUS_POWER_SW 32

namespace PM {

void enableBusPower();
void disableBusPower();

bool isPeriodicTaskRequested(uint32_t status);
void clearPeriodicTaskRequest(uint32_t &status);

bool isWakeVoltageSatisfied(uint32_t status);

bool isEmergencyTaskRequested(uint32_t status);
void clearEmergencyTaskRequest(uint32_t &status);

} // namespace PM
