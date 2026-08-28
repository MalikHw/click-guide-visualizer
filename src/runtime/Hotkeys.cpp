#include "Hotkeys.hpp"

#include "core/Compat.hpp"
#include "runtime/Runtime.hpp"
#include "settings/Settings.hpp"
#include "store/MacroCycle.hpp"
#include "ui/CornerToast.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

namespace cgv {

namespace {

constexpr cocos2d::ccColor3B kOnColour{120, 255, 170};
constexpr cocos2d::ccColor3B kOffColour{200, 200, 210};
constexpr cocos2d::ccColor3B kProblemColour{255, 130, 150};

bool inGameplay() {
    return PlayLayer::get() != nullptr;
}

void toggleGuide() {
    auto* mod = Mod::get();
    if (!mod) return;

    bool next = !mod->getSettingValue<bool>("show-guide");
    mod->setSettingValue<bool>("show-guide", next);

    SettingsCache::get().refresh();
    Runtime::get().requestRedraw();

    showToast(next ? "Guide on" : "Guide off", next ? kOnColour : kOffColour);
}

void switchMacro(int direction) {
    auto result = cycleMacro(direction);

    if (!result.ok) {
        showToast(result.message, kProblemColour);
        return;
    }

    Runtime::get().requestRedraw();
    showToast(result.message);
}

} // namespace

void installHotkeys() {
    listenForKeybindSettingPresses(
        kToggleKeySetting,
        [](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat || !inGameplay()) return false;
            toggleGuide();
            return true;
        });

    listenForKeybindSettingPresses(
        kPreviousKeySetting,
        [](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat || !inGameplay()) return false;
            switchMacro(-1);
            return true;
        });

    listenForKeybindSettingPresses(
        kNextKeySetting,
        [](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat || !inGameplay()) return false;
            switchMacro(1);
            return true;
        });
}

} // namespace cgv
