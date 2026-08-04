#pragma once

#include "core/Gamemode.hpp"

#include <array>
#include <cocos2d.h>

namespace cgv {

struct Snapshot {
    bool showGuide = true;
    cocos2d::ccColor3B lineColor{0, 255, 156};
    cocos2d::ccColor3B playerTwoColor{255, 150, 40};
    cocos2d::ccColor3B holdColor{0, 255, 156};
    cocos2d::ccColor3B markerColor{255, 255, 255};
    cocos2d::ccColor3B judgementLineColor{255, 255, 255};

    float indicatorOpacity = 0.75f;
    float bandOpacity = 0.55f;
    float fillOpacity = 0.35f;
    float holdOpacity = 1.f;
    float borderOpacity = 0.9f;
    float borderWidth = 1.5f;
    float lineOpacity = 0.85f;
    float lineWidth = 0.8f;

    bool showPlayerLine = true;
    bool showBandLines = true;
    float cullPadding = 0.f;
    bool showAccuracy = true;
    bool showClicks = true;
    float markerRow = 0.5f;
    bool learnTrajectory = true;
    bool predictHeights = false;
    bool showPath = false;

    std::array<bool, kGamemodeCount> modeEnabled{};

    float lineThickness = 2.5f;
    float markerSize = 7.f;
    bool showReleases = false;

    float lineLeadUnits = 0.f;
    float lineLeadSeconds = 0.f;
    bool clipBandsAtLine = false;

    bool rhythmMode = false;
    float rhythmLaneX = 0.5f;
    float rhythmLaneWidth = 220.f;
    float rhythmHitY = 0.5f;
    float rhythmLeadTime = 2.0f;
    float rhythmNoteThickness = 22.f;
    float rhythmLaneOpacity = 0.5f;
    bool rhythmHideWorldGuide = true;
    bool rhythmHitEffects = true;
    float rhythmHitEffectTime = 0.25f;

    bool generateFromLevel = false;
    float generateSpeed = 311.58f;

    bool enableCheats = false;
    bool assistBufferInput = false;
    float assistBufferSeconds = 0.25f;
    bool assistShowWindow = true;

    bool autoAlign = true;
    bool autoLoadMacro = true;
    bool debugLog = false;

    bool enabledFor(Gamemode mode) const {
        return modeEnabled[static_cast<size_t>(mode)];
    }
};

class SettingsCache {
public:
    static SettingsCache& get();

    void refresh();
    Snapshot const& snapshot() const { return m_snapshot; }

private:
    SettingsCache() = default;

    Snapshot m_snapshot;
};

Snapshot const& settings();

} // namespace cgv
