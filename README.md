<p align="center">
  <img align="center" height="300px" width="auto" src="./images/eee_main.gif">
  <p align="center">
  <small>The clip played is from The Melancholy of Haruhi Suzumiya (2009). </small>
  </p>
</p>

# Endless Endless Eight

A self-sustained AVI player to replay the summer days.

- ESP32 to play MJPEG-AVI video files from microSD at 24 fps.
- Utilizes ULP coprocessor for BAT voltage monitoring, Deep Sleep management etc.
- 5V solar panel and small LiPo battery as the main power source.
- (Planned) LTC3130-1 for more efficient power conversion.

## BOM

**WIP**

- ESP32 WeMos LOLIN32 Lite
- ST7735 80x160 LCD module
- microSD card (>= 2 GB)
- 5V 2W solar panel

## Prepare video files

All the video files to be placed on the MicroSD must be MJPEG-encoded AVI.

```sh
# Transcode EP 12~19 to ee_%d.avi, with scaling and cropping
$ seq 12 19 | xargs -L 1 -P1 -I% bash -c 'ffmpeg -i ee_%.mp4 -r 24 -vf scale=160:-1,crop=160:80 -vcodec mjpeg -q:v 5 -an ee_%.avi -y'
```

See [sdcard/](./sdcard/) for a set of sample vid files. These are converted from [BigBuckBunny.mp4](http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4).

```sh
# The sample vid files were from
$ seq 12 19 | xargs -L 1 -P1 -I% bash -c 'ffmpeg -ss $(((% - 12)*2)) -to $(((% - 12 + 1) * 2)) -i BigBuckBunny.mp4 -r 24 -vf scale=160:-1,crop=160:80 -vcodec mjpeg -q:v 5 -an ee_%.avi -y'
```

## Build firmware

```sh
$ git clone https://github.com/likeablob/endless-endless-eight.git
$ cd endless-endless-eight

$ cp include/user_config.g.template cp include/user_config.h
$ code include/user_config.h

$ pio run || pio run # The very 1st run may fail due to ulptool-pio
$ pio run -t upload
```

## Print enclosure

See [./enclosure](./enclosure).

## How it works

### State transitions

Centering on the deep sleep mode, this application has several wake-up states as depicted in the following diagram.

- The application switches back-and-forth between the nominal mode and deep sleep depending on the battery voltage. (See [Power management strategy](#power-management-strategy) for details.)
- Also, under the certain condition, it wakes in;
  - Emergency mode to prepare for a complete power loss (See [State persistence](#state-persistence))
  - Periodic Reporting mode for reporting housekeeping data via MQTT
- Basically every wake up transition is triggered by the ULP routine.

```mermaid
stateDiagram-v2
    Playing: Playing Video (Nominal)
    [*] --> Playing
    Playing --> DeepSleep: At low battery
    DeepSleep --> Playing: At high battery

    DeepSleep --> Emergency: At very low battery
    Emergency --> DeepSleep: ASAP

    DeepSleep --> PeriodicReporting: Every 30 minutes
    PeriodicReporting --> DeepSleep: ASAP
```

### Power management strategy

It's quite simple; Wake the Main CPU up if `the battery voltage (BAT_V) >= 3.9 V` and put into sleep if `BAT_V <= 3.75 V`.  
The ULP co-processor executes own routine every one second, but still is able to keep the deep sleep current consumption at uA-level.

```mermaid
flowchart TB
  subgraph CPU["Main CPU"]
    %% flow
    power_boot(" ")
    boot("Boot")
    check_ulp_boot{"Wake by ULP?"}
    init_ulp["Init ULP routine"]

    %% flow
    init_loop(" ")
    check_batv_sleep{"BAT_V <= SLEEP_TH"}
    sleep("Deep sleep")

    power_boot --> boot --> check_ulp_boot
    check_ulp_boot -->|"Yes"| init_loop
    check_ulp_boot -->|"No"| init_ulp --> init_loop

    init_loop --> check_batv_sleep
    check_batv_sleep -->|"Yes"| sleep
    check_batv_sleep -->|"No"| init_loop
    sleep -.-> |"Wake by ULP"| boot
  end

  subgraph ULP
    %% nodes
    boot_ulp(" ")
    measure_batv["Measure Battery voltage (BAT_V) by ADC"]
    check_batv_wake{"BAT_V >= WAKE_TH"}
    wake_main["Wake Main CPU"]
    end_ulp(" ")

    %% flow
    boot_ulp --> measure_batv --> check_batv_wake
    check_batv_wake -->|"Yes"| wake_main ---> end_ulp
    check_batv_wake --->|"No"| end_ulp

    end_ulp -.->|"Every 1 sec"| boot_ulp
  end

  %% flow
  init_ulp -.-> boot_ulp
```

### State persistence

The play state such as a file playing, its position and loop count etc. are kept over deep sleep or even complete power outage, thanks to ~~Nagato~~ [RTC SLOW MEM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html#overview) and [Non-Volatile Storage](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html). The former is volatile but survives deep sleep, and the latter is nothing but a reserved region in flash.

In short there are two pathways for each storage.

- A. Save to `RTC_SLOW_MEM` before entering deep sleep. Restore from `RTC_SLOW_MEM` after waking up from deep sleep.
- B. Save to `NVS` at very-low battery voltage. Restore from `NVS` on boot.

Note: About B., the ULP routine wakes the main CPU in the Emergency mode when `BAT_V <= 3.55 V`. This happens only once and the related flag will be cleared by the main CPU after normal wake up.

```mermaid
flowchart
  %% nodes
  init(" ")
  check_ulp_boot{"Wake by ULP?"}
  restore_from_nvs["Restore from Non-volatile storage"]
  restore_from_rtc["Restore from RTC SLOW RAM\n(Volatile, Deep-sleep persistent)"]
  check_ulp_flag_emergency{"Emergency Mode?\n(caused by very low battery)"}
  save_to_nvs["Save to Non-volatile storage"]
  sleep("Deep sleep")
  init_loop("Main Loop")
  check_low_bat{"Low battery?"}
  save_to_rtc["Save to RTC SLOW MEM"]

  init ---> check_ulp_boot
  check_ulp_boot -->|"Yes"| check_ulp_flag_emergency
  check_ulp_boot -->|"No"| restore_from_nvs --> init_loop
  check_ulp_flag_emergency --> |"Yes"| save_to_nvs --> sleep
  check_ulp_flag_emergency --> |"No"| restore_from_rtc --> init_loop

  init_loop --> check_low_bat
  check_low_bat --> |"Yes"| save_to_rtc --> sleep
  check_low_bat --> |"No"| init_loop

  sleep -.-> init

  classDef restore fill:#dce4ef,stroke:#333,stroke-width:2px;
  classDef save fill:#e7f4e4,stroke:#333,stroke-width:2px;
  class restore_from_nvs,restore_from_rtc restore
  class save_to_nvs,save_to_rtc save
```

## LICENSE

MIT

## Dependencies

This projects is here thanks to a lot of superb OSS libraries.
See [platformio.ini](./platformio.ini) for details. Thank you to all the devs.
