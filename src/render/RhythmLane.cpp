#include "RhythmLane.hpp"

#include "assist/AssistState.hpp"
#include "render/GuideNode.hpp"
#include "render/GuidePainter.hpp"
#include "runtime/Runtime.hpp"
#include "settings/Settings.hpp"

#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace geode::prelude;
using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kTrailingVisibleSeconds = 0.35f;

} // namespace

void RhythmLane::attach(PlayLayer* layer) {
    m_layer = layer;
    m_hitFlash = 0.f;
    m_effects.clear();
    ensureNodes();
}

void RhythmLane::detach() {
    m_labels.detach();
    m_effects.clear();
    m_node = nullptr;
    m_layer = nullptr;
    m_timeline.clear();
    m_consumed.clear();
}

void RhythmLane::ensureNodes() {
    if (!m_layer || m_node) return;

    m_node = GuideNode::create();
    if (!m_node) return;

    m_layer->addChild(m_node, kLaneZOrder);
    m_labels.attachTo(m_layer, kLaneLabelZOrder);
}

void RhythmLane::rebuild(std::vector<RhythmNote> timeline) {
    m_timeline = std::move(timeline);
    m_consumed.assign(m_timeline.size(), false);
}

void RhythmLane::clearConsumed() {
    m_consumed.assign(m_timeline.size(), false);
}

void RhythmLane::reset() {
    m_hitFlash = 0.f;
    m_effects.clear();
    clearConsumed();
}

void RhythmLane::registerHit() {
    m_hitFlash = 1.f;
}

std::vector<double> RhythmLane::hitTimesForPlayerOne() const {
    std::vector<double> times;
    times.reserve(m_timeline.size());
    for (auto const& note : m_timeline) {
        if (note.player2) continue;
        times.push_back(note.hitTime);
    }
    return times;
}

LaneLayout RhythmLane::currentLayout() const {
    return computeLaneLayout(CCDirector::sharedDirector()->getWinSize(), settings());
}

void RhythmLane::consumeNearestNote(double songTime, bool player2, int deltaFrames) {
    if (m_timeline.empty()) return;
    if (m_consumed.size() != m_timeline.size()) clearConsumed();

    size_t bestIndex = m_timeline.size();
    double bestDistance = std::numeric_limits<double>::max();

    size_t index = firstNoteAtOrAfter(m_timeline, songTime - kConsumeWindowSeconds);
    for (; index < m_timeline.size(); ++index) {
        auto const& note = m_timeline[index];
        if (note.hitTime > songTime + kConsumeWindowSeconds) break;
        if (m_consumed[index]) continue;
        if (note.player2 != player2) continue;

        double distance = std::fabs(note.hitTime - songTime);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = index;
        }
    }

    if (bestIndex >= m_timeline.size()) return;

    m_consumed[bestIndex] = true;

    auto const& config = settings();
    if (!config.rhythmHitEffects) return;

    LaneLayout layout = currentLayout();
    float screenX = projectToScreen(m_timeline[bestIndex].hitTime, songTime, layout);
    screenX = std::clamp(screenX, layout.laneLeft, layout.laneRight);

    m_effects.spawn(screenX, player2, deltaFrames, config.rhythmHitEffectTime);
}

float RhythmLane::projectToScreen(double noteTime, double songTime, LaneLayout const& layout) const {
    double remaining = noteTime - songTime;
    return layout.hitX + static_cast<float>(remaining) * m_pixelsPerSecond;
}

void RhythmLane::drawNotes(double songTime, LaneLayout const& layout) {
    auto const& config = settings();

    double windowStart = songTime - kTrailingVisibleSeconds;
    size_t index = firstNoteAtOrAfter(m_timeline, windowStart);
    double horizon = songTime + config.rhythmLeadTime;

    for (; index < m_timeline.size(); ++index) {
        auto const& note = m_timeline[index];
        if (note.hitTime > horizon) break;
        if (index < m_consumed.size() && m_consumed[index]) continue;

        float startX = projectToScreen(note.hitTime, songTime, layout);
        float endX = projectToScreen(note.releaseTime, songTime, layout);

        if (startX > layout.laneRight && endX > layout.laneRight) continue;
        if (endX < layout.laneLeft) continue;

        double approach = note.hitTime - songTime;
        float alphaScale = 1.f;
        if (approach > config.rhythmLeadTime - kNoteFadeInSeconds) {
            float span = std::max(kNoteFadeInSeconds, 0.001f);
            alphaScale = std::clamp(static_cast<float>((config.rhythmLeadTime - approach) / span), 0.f, 1.f);
        }

        RhythmPainter::paintNote(m_node, layout, config, startX, endX, note.isHold,
                                 note.player2, alphaScale);
    }
}

void RhythmLane::update(double songTime, float deltaSeconds) {
    if (!m_node) {
        ensureNodes();
        if (!m_node) return;
    }

    m_node->clear();

    auto const& config = settings();
    if (!config.showGuide || !config.rhythmMode) {
        m_node->setVisible(false);
        m_labels.beginFrame();
        m_labels.endFrame();
        return;
    }

    m_node->setVisible(true);

    if (m_hitFlash > 0.f) {
        m_hitFlash = std::max(0.f, m_hitFlash - deltaSeconds * kHitFlashDecayPerSecond);
    }

    m_effects.advance(deltaSeconds);

    CCSize windowSize = CCDirector::sharedDirector()->getWinSize();
    LaneLayout layout = computeLaneLayout(windowSize, config);

    float leadTime = std::max(config.rhythmLeadTime, 0.01f);
    m_pixelsPerSecond = (windowSize.width - layout.hitX) / leadTime;

    RhythmPainter::paintLaneBackground(m_node, layout, config);

    if (cheatsAreActive() && config.assistShowWindow) {
        RhythmPainter::paintTimingWindow(m_node, layout, config,
                                         config.assistBufferSeconds * m_pixelsPerSecond);
    }

    drawNotes(songTime, layout);
    RhythmPainter::paintHitLine(m_node, layout, config, m_hitFlash);
    m_effects.paint(m_node, layout, config);

    m_labels.beginFrame();
    if (config.showAccuracy) {
        for (auto const& judgement : Runtime::get().judgements()) {
            float life = judgement.remainingSeconds / kJudgementLifetimeSeconds;
            m_labels.place(layout.hitX, layout.centerY + layout.halfHeight + 6.f,
                           judgement.deltaFrames, life);
        }
    }
    m_labels.endFrame();
}

} // namespace cgv
