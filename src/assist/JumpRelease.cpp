#include "JumpRelease.hpp"

#include "assist/AssistState.hpp"

#include <Geode/Bindings.hpp>

namespace cgv {

namespace {

constexpr int kJumpButton = 1;

} // namespace

void releaseDueJumps(PlayLayer* layer) {
    if (!layer) return;

    auto& state = AssistState::get();
    if (!state.active()) return;

    bool player2 = false;
    while (state.buffer().takeDueJump(state.songTime(), player2)) {
        state.setReleasing(true);
        layer->handleButton(true, kJumpButton, !player2);
        layer->handleButton(false, kJumpButton, !player2);
        state.setReleasing(false);
    }
}

} // namespace cgv
