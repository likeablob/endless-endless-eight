#pragma once
#include <Arduino.h>

#define FS_NO_GLOBALS
#include "SD_MMC.h"
#include <SPI.h>
#include <SPIFFS.h>
#include <Wire.h>

#include <Chrono.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#include "AviParser.h"

#define EE_FPS 24
#define EE_MSEC_PER_FRAME (1000 / EE_FPS)

#define FILE_LOAD_FLAG_CLEARED 0x0
#define FILE_LOAD_FLAG_LOAD_AFTER_BOOT 0x1
#define FILE_LOAD_FLAG_NEXT_FILE_REQUESTED 0x2

enum class FileLoadReason : uint8_t {
    CLEARED,
    LOAD_AFTER_BOOT,
    NEXT_FILE_REQUESTED
};

class EEE_ {
  private:
    EEE_() = default;

    Chrono renderTask;
    TFT_eSPI tft;
    AviParser aviParser;

    const char *_getVidFile(uint16_t ind);
    static bool _tftOutputCallback(int16_t x, int16_t y, uint16_t w, uint16_t h,
                                   uint16_t *bitmap);
    void _requestFirstVidFile();

  public:
    size_t fileInd = 0;
    size_t frameInd = 0;
    size_t loopCount = 0; // Nagato
    FileLoadReason shouldLoadFile = FileLoadReason::LOAD_AFTER_BOOT;

    static EEE_ &getInstance(); // Accessor for singleton instance

    EEE_(const EEE_ &) = delete; // Prohibit copying
    EEE_ &operator=(const EEE_ &) = delete;

    bool begin();
    void handle();
    void renderFrame();
};

extern EEE_ &EEE;
