#include "stash.h"

Preferences preferences;

namespace Stash {

bool save(uint32_t fileInd, uint32_t frameInd, uint32_t loopCount) {
    if(!preferences.begin(STASH_PREFERENCE_NS, false)) {
        return false;
    }

    preferences.putUInt(STASH_KEY_FILE_IND, fileInd);
    preferences.putUInt(STASH_KEY_FRAME_IND, frameInd);
    preferences.putUInt(STASH_KEY_LOOP_COUNT, loopCount);

    preferences.end();
}

bool restore(uint32_t &fileInd, uint32_t &frameInd, uint32_t &loopCount) {
    if(!preferences.begin(STASH_PREFERENCE_NS, true)) {
        return false;
    }

    fileInd = preferences.getUInt(STASH_KEY_FILE_IND, 0);
    frameInd = preferences.getUInt(STASH_KEY_FRAME_IND, 0);
    loopCount = preferences.getUInt(STASH_KEY_LOOP_COUNT, 0);

    preferences.end();
}

} // namespace Stash
