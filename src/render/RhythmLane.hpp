#pragma once

#include "render/HitEffects.hpp"
#include "render/JudgementLabels.hpp"
#include "render/RhythmModel.hpp"
#include "render/RhythmPainter.hpp"

#include <cocos2d.h>
#include <cstddef>
#include <vector>

class PlayLayer;

namespace cgv {

class GuideNode;

constexpr int kLaneZOrder = 1500;
constexpr int kLaneLabelZOrder = 1501;
constexpr float kHitFlashDecayPerSecond = 6.f;
constexpr float kNoteFadeInSeconds = 0.15f;
constexpr double kConsumeWindowSeconds = 0.35;

class RhythmLane {
public:
    void attach(PlayLayer* layer);
    void detach();

    void rebuild(std::vector<RhythmNote> timeline);
    void reset();
    void clearConsumed();

    void update(double songTime, float deltaSeconds);
    void consumeNearestNote(double songTime, bool player2, int deltaFrames);
    void registerHit();

    std::vector<double> hitTimesForPlayerOne() const;
    bool nearestNoteTime(double songTime, bool player2, double& timeOut) const;
    bool hasTimeline() const { return !m_timeline.empty(); }

private:
    void ensureNodes();
    void drawNotes(double songTime, LaneLayout const& layout);
    float projectToScreen(double noteTime, double songTime, LaneLayout const& layout) const;
    LaneLayout currentLayout() const;

    PlayLayer* m_layer = nullptr;
    GuideNode* m_node = nullptr;
    JudgementLabels m_labels;
    HitEffects m_effects;

    std::vector<RhythmNote> m_timeline;
    std::vector<bool> m_consumed;
    float m_hitFlash = 0.f;
    float m_pixelsPerSecond = 0.f;
};

} // namespace cgv
