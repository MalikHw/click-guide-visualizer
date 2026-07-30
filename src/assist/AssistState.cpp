#include "AssistState.hpp"

#include "settings/Settings.hpp"

namespace cgv {

AssistState& AssistState::get() {
    static AssistState instance;
    return instance;
}

bool cheatsAreActive() {
    auto const& config = settings();
    if (!config.enableCheats) return false;
    return config.assistBufferInput;
}

} // namespace cgv
