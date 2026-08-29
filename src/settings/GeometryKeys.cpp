#include "GeometryKeys.hpp"

#include <algorithm>
#include <array>

namespace cgv {

namespace {

constexpr std::array<std::string_view, 27> kAppearanceOnlyKeys = {
    "line-color",
    "player2-color",
    "hold-color",
    "marker-color",
    "judgement-line-color",
    "indicator-opacity",
    "band-opacity",
    "fill-opacity",
    "hold-opacity",
    "border-opacity",
    "border-width",
    "line-opacity",
    "line-width",
    "line-thickness",
    "marker-size",
    "cull-padding",
    "show-accuracy",
    "show-clicks",
    "show-band-lines",
    "show-wave-path",
    "rhythm-lane-x",
    "rhythm-lane-width",
    "rhythm-hit-y",
    "rhythm-note-thickness",
    "rhythm-lane-opacity",
    "hotkey-popup-seconds",
    "debug-log",
};

} // namespace

bool settingChangesGeometry(std::string_view key) {
    return std::find(kAppearanceOnlyKeys.begin(), kAppearanceOnlyKeys.end(), key) ==
           kAppearanceOnlyKeys.end();
}

} // namespace cgv
