#pragma once

#include <FS.h>
#include <stdint.h>

#define FOURCC_RIFF ('R' | 'I' << 8 | 'F' << 16 | 'F' << 24)
#define FOURCC_AVI ('A' | 'V' << 8 | 'I' << 16 | ' ' << 24)
#define FOURCC_LIST ('L' | 'I' << 8 | 'S' << 16 | 'T' << 24)
#define FOURCC_MOVI ('m' | 'o' << 8 | 'v' << 16 | 'i' << 24)
#define FOURCC_IDX1 ('i' | 'd' << 8 | 'x' << 16 | '1' << 24)
#define SIZE_FOURCC 4

typedef uint8_t avi_parser_err_t;

#define AVI_PARSER_OK 0x0
#define AVI_PARSER_LAST_FRAME 0x10
#define AVI_PARSER_ERR 0x20
#define AVI_PARSER_ERR_SEEK_FAILED 0x30
#define AVI_PARSER_ERR_READ_FAILED 0x40
#define AVI_PARSER_ERR_INVALID_CHUNK 0x50
#define AVI_PARSER_ERR_FILE_NOT_FOUND 0x60
#define AVI_PARSER_ERR_NOT_SUPPORTED 0x70
#define AVI_PARSER_ERR_MOVI_NOT_FOUND 0x80
#define AVI_PARSER_ERR_IDX1_NOT_FOUND 0x90

typedef struct {
    uint32_t dwFourCC;
    uint32_t dwSize;
} AviChunk;

typedef struct {
    uint32_t dwList;
    uint32_t dwSize;
    uint32_t dwFourCC;
} AviList;

typedef struct {
    uint32_t ckid;
    uint32_t dwFlags;
    uint32_t dwChunkOffset;
    uint32_t dwChunkLength;
} AviIndexData;

class AviParser {
  private:
    // Have two separated file handlers for the same file.
    // Dirty hack to speed up `fseek` calls on reading each frame.
    fs::File _vidFileFrame;
    fs::File _vidFileIndex;

  public:
    uint32_t posMoviChunkDataStart = 0;
    uint32_t posIdx1ChunkDataStart = 0;
    AviParser();
    ~AviParser();

    avi_parser_err_t init(fs::FS &fs, const char *path);

    avi_parser_err_t readIndex(uint32_t ind, AviIndexData &out);
    avi_parser_err_t readFrame(AviIndexData &indexData, uint8_t *out);
};
