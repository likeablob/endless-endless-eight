#ifdef _ULPCC_ // do not add code above this line
#include "ulp_common.h"
#include <ulp_c.h>

#define ADC_CHANNEL 6              // ADC1 Channel 6, GPIO34
#define COUNTER_PERIODIC_TASK 1800 // 1800 sec = 0.5 hour

// Global variables
unsigned _counterPeriodicTask = 1780;
unsigned status = 0;
unsigned batV = 0;

void entry() {
    _counterPeriodicTask++;
    if(_counterPeriodicTask >= COUNTER_PERIODIC_TASK) {
        _counterPeriodicTask = 0;
        status |= PM_STATUS_PERIODIC_TASK_REQ;
        wake();
        return;
    }

    // Measure batV and apply LPF (k=0.5)
    if(batV == 0) {
        batV = adc(0, ADC_CHANNEL + 1); // init
    }
    batV = (adc(0, ADC_CHANNEL + 1) >> 1) + (batV >> 1);

    // Wake if batV is above the threshold
    if(batV >= PM_VBAT_TH_WAKE) {
        status |= PM_STATUS_WAKE_VOLTAGE_SATISFIED;
        wake();
        return;
    }

    // Wake only once if batV is under the threshold
    if(batV <= PM_VBAT_TH_EMERGENCY &&
       !(status & PM_STATUS_EMERGENCY_TASK_REQ)) {
        // This flag will be cleared by the main code when it's woken with
        // PM_STATUS_WAKE_VOLTAGE_SATISFIED.
        status |= PM_STATUS_EMERGENCY_TASK_REQ;
        wake();
        return;
    }
}
#endif // do not add code after here
