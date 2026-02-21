#include "state.h"

namespace app {

void addEvent(SystemState &state, const String &text) {
    EventRecord &slot = state.eventLog[state.eventHead];
    slot.ts = millis();
    slot.text = text;

    state.eventHead = (state.eventHead + 1) % APP_EVENT_LOG_CAPACITY;
    if (state.eventCount < APP_EVENT_LOG_CAPACITY) {
        state.eventCount++;
    }
}

} // namespace app
