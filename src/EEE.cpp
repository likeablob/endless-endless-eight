#include "EEE.h"

const char *vidFiles[] = {"/ee_12.avi", "/ee_13.avi", "/ee_14.avi",
                          "/ee_15.avi", "/ee_16.avi", "/ee_17.avi",
                          "/ee_18.avi", "/ee_19.avi"};
const uint16_t vidFilesLen = sizeof(vidFiles) / sizeof(vidFiles[0]);

EEE_ &EEE_::getInstance() {
    static EEE_ instance;
    return instance;
}

EEE_ &EEE = EEE.getInstance();

const char *EEE_::_getVidFile(uint16_t ind) {
    return vidFiles[ind % vidFilesLen];
}

bool EEE_::_tftOutputCallback(int16_t x, int16_t y, uint16_t w, uint16_t h,
                              uint16_t *bitmap) {
    // Stop further decoding as image is running off bottom of screen
    if(y >= EEE.tft.height())
        return 0;

    EEE.tft.pushImage(x, y, w, h, bitmap);

    // Decode next block
    return 1;
}

void EEE_::_requestFirstVidFile() {
    fileInd = 0;
    frameInd = 0;
    shouldLoadFile = FileLoadReason::NEXT_FILE_REQUESTED;
}

bool EEE_::begin() {
    // Init SDMMC
    if(!SD_MMC.begin()) {
        Serial.println("Card Mount Failed");
        return false;
    }

    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE) {
        Serial.println("No SD_MMC card attached");
        return false;
    }

    // Init TFT
    tft.init();             // initialize a ST7735 (80x160), see platformio.ini
    tft.setSwapBytes(true); // Swap the colour bytes (endianess)
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    // Init JPG decoder
    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(this->_tftOutputCallback);

    // Init periodic tasks
    renderTask.start();

    return true;
}

bool EEE_::handle() {
    // Load the next vid file if requested.
    if(shouldLoadFile != FileLoadReason::CLEARED) {
        // Preserve preloaded frameInd on LOAD_AFTER_BOOT
        frameInd =
            (shouldLoadFile == FileLoadReason::LOAD_AFTER_BOOT) ? frameInd : 0;

        const char *path = _getVidFile(fileInd);
        Serial.printf("Loading a file: %s\n", path);

        uint32_t tInit = millis();
        avi_parser_err_t err = aviParser.init(SD_MMC, path);
        if(err) {
            Serial.printf("An error returned by aviParser.init(): 0x%x\r\n",
                          err);
            fileInd = 0;
            frameInd = 0;
            // Keep shouldLoadFile to reload the file in the next loop.
            return false;
        }
        Serial.printf("tInit: %lu ms\r\n", millis() - tInit);
        shouldLoadFile = FileLoadReason::CLEARED;
    }

    if(renderTask.hasPassed(EE_MSEC_PER_FRAME)) {
        renderTask.restart();
        return renderFrame();
    }

    return true;
}

bool EEE_::renderFrame() {
    uint32_t tLoad = millis();

    // Load index data
    AviIndexData indexData;
    avi_parser_err_t err;

    err = aviParser.readIndex(frameInd, indexData);
    if(err == AVI_PARSER_LAST_FRAME) {
        Serial.printf("Reached to the end. fileInd: %u\n", fileInd);
        fileInd = (fileInd + 1) % vidFilesLen;
        shouldLoadFile =
            FileLoadReason::NEXT_FILE_REQUESTED; // Load the next file in the
                                                 // next loop
        loopCount++;
    } else if(err != AVI_PARSER_OK) {
        Serial.printf("An error returned by readIndex(): 0x%x\r\n", err);
        _requestFirstVidFile();
        return false;
    }

    // Load frame data
    uint8_t *imgData = (uint8_t *)malloc(indexData.dwChunkLength);
    err = aviParser.readFrame(indexData, imgData);
    if(err) {
        Serial.printf("An error returned by readFrame(): 0x%x\r\n", err);
        free(imgData);
        _requestFirstVidFile();
        return false;
    }
    tLoad = millis() - tLoad;

    // Draw the frame
    uint32_t tRender = millis();
    TJpgDec.drawJpg(0, 0, imgData, indexData.dwChunkLength);
    free(imgData);
    tRender = millis() - tRender;

    // Check if it's speedy enough
    if(renderTask.elapsed() > EE_MSEC_PER_FRAME) {
        Serial.printf("Running late! elapsed: %lu ms, frameInd: %u \r\n",
                      renderTask.elapsed(), frameInd);
        Serial.printf("tLoad: %u ms, tRender: %u ms\r\n", tLoad, tRender);
    }

    frameInd++;

    return true;
}
