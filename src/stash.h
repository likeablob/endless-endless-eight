#include <Preferences.h>
#include <stdint.h>

#define STASH_PREFERENCE_NS "eee"
#define STASH_KEY_FILE_IND "fileInd"
#define STASH_KEY_FRAME_IND "frameInd"
#define STASH_KEY_LOOP_COUNT "loopCount"

namespace Stash {

bool save(uint32_t fileInd, uint32_t frameInd, uint32_t loopCount);
bool restore(uint32_t &fileInd, uint32_t &frameInd, uint32_t &loopCount);

} // namespace Stash
