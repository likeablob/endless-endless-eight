#include "AviParser.h"

AviParser::AviParser(){};

AviParser::~AviParser() {
    this->_vidFileFrame.close();
    this->_vidFileIndex.close();
}

avi_parser_err_t AviParser::init(fs::FS &fs, const char *path) {
    this->_vidFileFrame.close();
    this->_vidFileIndex.close();
    this->posMoviChunkDataStart = 0;
    this->posIdx1ChunkDataStart = 0;

    if(!fs.exists(path)) {
        return AVI_PARSER_ERR_FILE_NOT_FOUND;
    }

    fs::File file = fs.open(path, "r");

    // Ensure this is an AVI file by reading the RIFF chunk
    AviList riff = {};
    file.readBytes((char *)&riff, sizeof(AviList));
    if(riff.dwList != FOURCC_RIFF || riff.dwFourCC != FOURCC_AVI) {
        file.close();
        return AVI_PARSER_ERR_NOT_SUPPORTED;
    }

    // Locate required chunks
    while(file.peek() != -1) {
        AviChunk chunk = {};
        file.readBytes((char *)&chunk, sizeof(AviChunk));

        if(chunk.dwFourCC == FOURCC_LIST) {
            uint32_t listFourCC = 0;
            file.readBytes((char *)&listFourCC, sizeof(uint32_t));
            file.seek(file.position() -
                      sizeof(uint32_t)); // Go back a little bit

            // movi chunk
            if(listFourCC == FOURCC_MOVI) {
                this->posMoviChunkDataStart = file.position();
            }
        } else if(chunk.dwFourCC == FOURCC_IDX1) {
            // idx1 chunk
            this->posIdx1ChunkDataStart = file.position();
        }

        // Skip data
        file.seek(chunk.dwSize, SeekMode::SeekCur);
    }

    if(!this->posMoviChunkDataStart) {
        file.close();
        return AVI_PARSER_ERR_MOVI_NOT_FOUND;
    }

    if(!this->posIdx1ChunkDataStart) {
        file.close();
        return AVI_PARSER_ERR_IDX1_NOT_FOUND;
    }

    this->_vidFileIndex = file;
    this->_vidFileFrame = fs.open(path);

    this->_vidFileIndex.seek(this->posIdx1ChunkDataStart, SeekMode::SeekSet);
    this->_vidFileFrame.seek(this->posMoviChunkDataStart, SeekMode::SeekSet);

    return AVI_PARSER_OK;
}

avi_parser_err_t AviParser::readIndex(uint32_t ind, AviIndexData &out) {
    fs::File file = (this->_vidFileIndex);
    uint32_t offset = ind * sizeof(AviIndexData);

    // Seek to the specified index in the idx1 chunk
    bool ok =
        file.seek(this->posIdx1ChunkDataStart + offset, SeekMode::SeekSet);
    if(!ok) {
        return AVI_PARSER_ERR_SEEK_FAILED;
    }

    // Read the index data
    uint32_t readLen = file.readBytes((char *)&out, sizeof(AviIndexData));
    if(readLen != sizeof(AviIndexData)) {
        return AVI_PARSER_ERR_READ_FAILED;
    }

    // Assume that 'idx1' is the last chunk in the file
    if(-1 == file.peek()) {
        return AVI_PARSER_LAST_FRAME;
    }

    return AVI_PARSER_OK;
}

avi_parser_err_t AviParser::readFrame(AviIndexData &indexData, uint8_t *out) {
    fs::File file = (this->_vidFileFrame);

    // Seek to the movi chunk
    bool ok = file.seek(this->posMoviChunkDataStart + indexData.dwChunkOffset,
                        SeekMode::SeekSet);
    if(!ok) {
        return AVI_PARSER_ERR_SEEK_FAILED;
    }

    // Read the chunk
    AviChunk chunk = {};
    uint32_t readLen = file.readBytes((char *)&chunk, sizeof(AviChunk));
    if(chunk.dwSize != indexData.dwChunkLength) {
        return AVI_PARSER_ERR_INVALID_CHUNK;
    }
    readLen += file.readBytes((char *)out, chunk.dwSize);

    // If readBytes failed
    if(readLen != (indexData.dwChunkLength + sizeof(AviChunk))) {
        return AVI_PARSER_ERR_READ_FAILED;
    }

    return AVI_PARSER_OK;
}
