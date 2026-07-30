#pragma once

#include "core/Gamemode.hpp"
#include "render/GuideGeometry.hpp"
#include "render/GuidePainter.hpp"
#include "render/HeightModel.hpp"
#include "render/JudgementLabels.hpp"
#include "render/PhysicsPredictor.hpp"
#include "render/RhythmLane.hpp"
#include "render/RhythmModel.hpp"
#include "level/LevelScan.hpp"
#include "runtime/Alignment.hpp"

#include <cocos2d.h>
#include <cstdint>

class PlayLayer;
class PlayerObject;

namespace cgv {

class GuideNode;

constexpr int kBandZOrder = 1000;
constexpr int kCursorZOrder = 1001;
constexpr int kLabelZOrder = 1002;
constexpr float kSpawnLatchSeconds = 0.25f;

class GuideController {
public:
    void attach(PlayLayer* layer);
    void detach();

    void onLevelReset();
    void onPostUpdate(float deltaSeconds);
    void onSettingsChanged();

private:
    void ensureNodes();
    void rebuildGeometry();
    void invalidateDerivedState();

    void latchSpawnAnchor();
    void consumePresses();
    void updateJudgements(float deltaSeconds);
    void updatePredictions();
    void observeHeights();

    void redraw();
    void rebuildRhythmTimeline();
    void rebuildFromLevelScan();
    void publishAssistTargets();
    bool worldGuideVisible() const;
    void drawBands(Viewport const& view);
    void drawMarkers(Viewport const& view);
    void drawLabels();

    Viewport computeViewport() const;
    float resolveMarkerY(Marker const& marker, Viewport const& view);
    double currentMacroFrame() const;
    bool gamemodeAllowed() const;

    PlayLayer* m_layer = nullptr;
    GuideNode* m_bandNode = nullptr;
    GuideNode* m_cursorNode = nullptr;
    JudgementLabels m_labels;

    Geometry m_geometry;
    Alignment m_alignment;
    HeightModel m_heights;
    PhysicsPredictor m_predictor;
    PhysicsPredictor m_predictorPlayer2;
    RhythmLane m_rhythmLane;
    ScannedLevel m_scannedLevel;
    bool m_levelScanned = false;

    uint64_t m_seenRevision = 0;
    float m_anchorX = 0.f;
    bool m_anchorLatched = false;
    bool m_geometryValid = false;
};

} // namespace cgv
