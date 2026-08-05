#include "assist/AssistState.hpp"
#include "runtime/Runtime.hpp"
#include "settings/Settings.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

namespace {

constexpr int kJumpButton = 1;

bool shouldHoldForLater(PlayLayer* playLayer, bool isPlayer1) {
    if (!cgv::cheatsAreActive()) return false;

    auto& state = cgv::AssistState::get();
    if (!state.active() || state.releasing()) return false;
    if (!state.buffer().hasTargets()) return false;

    bool held = state.buffer().tryBuffer(state.songTime(),
                                         cgv::settings().assistBufferSeconds,
                                         !isPlayer1);
    if (held) state.noteBuffered();
    return held;
}

} // namespace

struct GuideInputLayer : geode::Modify<GuideInputLayer, GJBaseGameLayer> {
    void handleButton(bool down, int button, bool isPlayer1) {
        auto* playLayer = typeinfo_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(this));

        if (down && button == kJumpButton && playLayer && shouldHoldForLater(playLayer, isPlayer1)) {
            return;
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer1);

        if (!down || button != kJumpButton || !playLayer) return;
        if (!cgv::settings().showGuide) return;

        cgv::Runtime::get().queuePress(!isPlayer1);
    }
};
