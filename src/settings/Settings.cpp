#include "Settings.hpp"

#include <Geode/loader/Mod.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace cgv {

namespace {

float clampUnit(double value) {
    if (!std::isfinite(value)) return 0.f;
    return static_cast<float>(std::clamp(value, 0.0, 1.0));
}

float clampRange(double value, double low, double high, double fallback) {
    if (!std::isfinite(value)) return static_cast<float>(fallback);
    return static_cast<float>(std::clamp(value, low, high));
}

bool boolSetting(const char* key, bool fallback) {
    auto* mod = Mod::get();
    if (!mod) return fallback;
    return mod->getSettingValue<bool>(key);
}

double floatSetting(const char* key, double fallback) {
    auto* mod = Mod::get();
    if (!mod) return fallback;
    return static_cast<double>(mod->getSettingValue<double>(key));
}

} // namespace

SettingsCache& SettingsCache::get() {
    static SettingsCache instance;
    return instance;
}

void SettingsCache::refresh() {
    auto* mod = Mod::get();
    if (!mod) return;

    Snapshot next;

    next.showGuide = boolSetting("show-guide", true);
    next.lineColor = mod->getSettingValue<ccColor3B>("line-color");
    next.playerTwoColor = mod->getSettingValue<ccColor3B>("player2-color");
    next.holdColor = mod->getSettingValue<ccColor3B>("hold-color");
    next.markerColor = mod->getSettingValue<ccColor3B>("marker-color");
    next.judgementLineColor = mod->getSettingValue<ccColor3B>("judgement-line-color");

    next.indicatorOpacity = clampUnit(floatSetting("indicator-opacity", 0.75));
    next.bandOpacity = clampUnit(floatSetting("band-opacity", 0.55));
    next.fillOpacity = clampUnit(floatSetting("fill-opacity", 0.35));
    next.holdOpacity = clampUnit(floatSetting("hold-opacity", 1.0));
    next.borderOpacity = clampUnit(floatSetting("border-opacity", 0.9));
    next.borderWidth = clampRange(floatSetting("border-width", 1.5), 0.0, 10.0, 1.5);
    next.lineOpacity = clampUnit(floatSetting("line-opacity", 0.85));
    next.lineWidth = clampRange(floatSetting("line-width", 0.8), 0.05, 10.0, 0.8);

    next.showPlayerLine = boolSetting("show-player-line", true);
    next.showBandLines = boolSetting("show-band-lines", true);
    next.cullPadding = clampRange(floatSetting("cull-padding", 0.0), 0.0, 2000.0, 0.0);
    next.showAccuracy = boolSetting("show-accuracy", true);
    next.showClicks = boolSetting("show-clicks", true);
    next.markerRow = clampUnit(floatSetting("marker-row", 0.5));
    next.learnTrajectory = boolSetting("learn-trajectory", true);
    next.predictHeights = boolSetting("predict-heights", false);
    next.showPath = boolSetting("show-path", false);

    for (size_t index = 0; index < kGamemodeCount; ++index) {
        auto mode = static_cast<Gamemode>(index);
        next.modeEnabled[index] = boolSetting(gamemodeSettingKey(mode), true);
    }

    next.lineThickness = clampRange(floatSetting("line-thickness", 2.5), 0.1, 30.0, 2.5);
    next.markerSize = clampRange(floatSetting("marker-size", 7.0), 1.0, 60.0, 7.0);
    next.showReleases = boolSetting("show-releases", false);

    next.lineLeadUnits = clampRange(floatSetting("line-lead-units", 0.0), 0.0, 600.0, 0.0);
    next.lineLeadSeconds = clampRange(floatSetting("line-lead-seconds", 0.0), 0.0, 3.0, 0.0);
    next.clipBandsAtLine = boolSetting("clip-bands-at-line", false);

    next.rhythmMode = boolSetting("rhythm-mode", false);
    next.rhythmLaneX = clampUnit(floatSetting("rhythm-lane-x", 0.5));
    next.rhythmLaneWidth = clampRange(floatSetting("rhythm-lane-width", 220.0), 60.0, 700.0, 220.0);
    next.rhythmHitY = clampRange(floatSetting("rhythm-hit-y", 0.5), 0.05, 0.95, 0.5);
    next.rhythmLeadTime = clampRange(floatSetting("rhythm-lead-time", 2.0), 0.25, 8.0, 2.0);
    next.rhythmNoteThickness = clampRange(floatSetting("rhythm-note-thickness", 5.0), 1.0, 40.0, 5.0);
    next.rhythmLaneOpacity = clampUnit(floatSetting("rhythm-lane-opacity", 0.5));
    next.rhythmHideWorldGuide = boolSetting("rhythm-hide-world-guide", true);
    next.rhythmHitEffects = boolSetting("rhythm-hit-effects", true);
    next.rhythmHitEffectTime = clampRange(floatSetting("rhythm-hit-effect-time", 0.25), 0.05, 1.0, 0.25);


    next.enableCheats = boolSetting("enable-cheats", false);
    next.assistBufferInput = boolSetting("assist-buffer-input", false);
    next.assistBufferSeconds = clampRange(floatSetting("assist-buffer-window", 0.25), 0.02, 1.0, 0.25);
    next.assistShowWindow = boolSetting("assist-show-window", true);

    next.autoAlign = boolSetting("auto-align", true);
    next.autoLoadMacro = boolSetting("auto-load-macro", true);
    next.onlineBrowsing = boolSetting("online-browsing", false);
    next.onlineAutoOpen = boolSetting("online-auto-open", false);
    next.hotkeyPopupSeconds = clampRange(floatSetting("hotkey-popup-seconds", 1.6), 0.4, 5.0, 1.6);
    next.debugLog = boolSetting("debug-log", false);

    m_snapshot = next;
}

Snapshot const& settings() {
    return SettingsCache::get().snapshot();
}

} // namespace cgv
