#include <Arduino.h>
#include <driver/adc.h>
#include <esp32/ulp.h>

// Third-party libs
#include <Chrono.h>
#include <ulptool.h>

// Special header generated by ulptool
#include "ulp_main.h"

// Local headers
#include "EEE.h"
#include "mqtt_reporter.h"
#include "pm.h"
#include "stash.h"
#include "ulp_common.h"

#define ULP_WAKEUP_PERIOD_US (1000 * 1000)
#define POWER_MANAGEMENT_TASK_POLL_MS 500

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

// Nagato
RTC_DATA_ATTR uint32_t RTC_EEE_fileInd = 0;
RTC_DATA_ATTR uint32_t RTC_EEE_frameInd = 0;
RTC_DATA_ATTR uint32_t RTC_EEE_loopCount = 0;

Chrono powerManagerTask;

static void init_ulp_adc() {
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_ulp_enable();
}

static void init_run_ulp(uint32_t usec) {
    ulp_set_wakeup_period(0, usec);
    esp_deep_sleep_disable_rom_logging(); // suppress boot messages

    esp_err_t err = ulptool_load_binary(
        0, ulp_main_bin_start,
        (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    err = ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t));

    init_ulp_adc();

    if(err) {
        Serial.println("Error Starting ULP Coprocessor");
    }

    esp_sleep_enable_ulp_wakeup();
}

void start_deep_sleep() {
    // Save variables into RTC SLOW MEM
    RTC_EEE_fileInd = EEE.fileInd;
    RTC_EEE_frameInd = EEE.frameInd;
    RTC_EEE_loopCount = EEE.loopCount;

    PM::disableBusPower();
    PM::clearWakeVoltageSatisfied(ulp_status);
    esp_deep_sleep_start();
}

void report(bool isRunning) {
#ifdef USE_MQTT_REPORTER
    uint16_t batVraw = (ulp_batV & 0xFFFF);
    uint16_t batV = (float)batVraw * 1.807f; // 1/4095 * 3700 * 2

    mqtt_data_map_t dataMap{
        {"millis", String(millis())},
        {"batV", String(batV)},
        {"batVraw", String(batVraw)},
        {"isRunning", String((uint8_t)isRunning)},
    };

    if(isRunning) {
        // No need to send loopCount if it's periodic wake (isRunning == false)
        dataMap.insert(std::make_pair("loopCount", String(EEE.loopCount)));
    }

    MqttReporter::report(dataMap);
#endif
}

void setup() {
    PM::disableBusPower();
    Serial.begin(921600);
    Serial.println("Started");

    MqttReporter::begin();

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if(cause != ESP_SLEEP_WAKEUP_ULP) {
        Serial.println("Not ULP wakeup, initializing ULP");
        init_run_ulp(ULP_WAKEUP_PERIOD_US);
        // Restore variables from non-volatile memory
        Stash::restore(EEE.fileInd, EEE.frameInd, EEE.loopCount);
        Serial.printf(
            "Restored from Stash: fileInd: %u, frameInd: %u, loopCount: %u\n",
            EEE.fileInd, EEE.frameInd, EEE.loopCount);
    } else {
        Serial.printf("ULP wakeup: batV: %u\n", (ulp_batV & 0xFFFF));
        esp_sleep_enable_ulp_wakeup();
        init_ulp_adc(); // FIXME: This keeps ESP32 stably wake-able even after
                        // esp_wifi_stop() is called.

        // If ULP requested to run the periodic task, run it and back to sleep
        // immediately.
        if(PM::isPeriodicTaskRequested(ulp_status)) {
            Serial.println("PeriodicTask is requested at wake");
            PM::clearPeriodicTaskRequest(ulp_status);
            report(false);
            esp_deep_sleep_start();
        }

        // If it's normal wake up caused by BAT voltage above the threshold.
        if(PM::isWakeVoltageSatisfied(ulp_status)) {
            PM::clearEmergencyTaskRequest(ulp_status);
        }

        // If it's emergency wake up where BAT voltage is below the threshold.
        if(PM::isEmergencyTaskRequested(ulp_status)) {
            // Save variables to non-volatile memory
            Serial.println("Emergency: Saving to Stash.");
            Stash::save(RTC_EEE_fileInd, RTC_EEE_frameInd, RTC_EEE_loopCount);
            esp_deep_sleep_start();
        }

        // Restore variables from RTC SLOW MEM
        EEE.fileInd = RTC_EEE_fileInd;
        EEE.frameInd = RTC_EEE_frameInd;
        EEE.loopCount = RTC_EEE_loopCount;
        Serial.printf("Restored: fileInd: %u, frameInd: %u, loopCount: %u\n",
                      EEE.fileInd, EEE.frameInd, EEE.loopCount);
    }

    PM::enableBusPower();

    if(!EEE.begin()) {
        Serial.println("EEE.begin() returned an error. Entering deep sleep.");
        start_deep_sleep();
    }
}

void loop() {
    if(PM::isPeriodicTaskRequested(ulp_status)) {
        PM::clearPeriodicTaskRequest(ulp_status);
        Serial.println("PeriodicTask is requested.");
        report(true);
    }

    if(powerManagerTask.hasPassed(POWER_MANAGEMENT_TASK_POLL_MS)) {
        powerManagerTask.restart();
        uint32_t batV = (ulp_batV & 0xFFFF);
        if(batV <= PM_VBAT_TH_SLEEP) {
            Serial.printf("Entering deep sleep. batV: %u\n", batV);
            start_deep_sleep();
        }
    }

    // Render a video frame
    if(!EEE.handle()) {
        Serial.println("EEE.handle() returned an error. Entering deep sleep.");
        start_deep_sleep();
    }
}
