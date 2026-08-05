#include "GuideController.hpp"

#include "core/Compat.hpp"
#include "parse/Framerate.hpp"
#include "render/GuideNode.hpp"
#include "render/GuidePainter.hpp"
#include "runtime/Runtime.hpp"
#include "settings/Settings.hpp"
#include "store/MacroStore.hpp"
#include "assist/AssistState.hpp"
#include "level/NoteGenerator.hpp"
#include "level/ModeProfile.hpp"

#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

using namespace geode::prelude;
using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kBandVerticalMargin = 4000.f;
constexpr float kViewportSlack = 240.f;
constexpr float kMarkerRowSpan = 120.f;
constexpr float kPlayerTwoMarkerOffset = 34.f;
constexpr float kDefaultSpeedX = 311.58f;

float playerSpeedX(PlayerObject* player) {
    if (!player) return kDefaultSpeedX;
    float speed = static_cast<float>(player->m_platformerXVelocity);
    if (!std::isfinite(speed) || std::fabs(speed) < 1.f) return kDefaultSpeedX;
    return std::fabs(speed);
}

} // namespace

void GuideController::attach(PlayLayer* layer) {
    m_layer = layer;
    m_seenRevision = 0;
    m_geometryValid = false;
    m_anchorLatched = false;
    m_anchorX = 0.f;

    m_alignment.reset();
    publishAlignmentReport();
    m_heights.clearAll();
    m_predictor.clear();
    m_predictorPlayer2.clear();

    Runtime::get().clearPresses();
    Runtime::get().clearJudgements();

    ensureNodes();
    m_rhythmLane.attach(layer);
    m_scannedLevel.clear();
    m_levelScanned = false;
    AssistState::get().setActive(true);
    AssistState::get().resetCounters();
}

void GuideController::detach() {
    AssistState::get().setActive(false);
    AssistState::get().buffer().clearTargets();
    m_rhythmLane.detach();
    m_labels.detach();
    m_bandNode = nullptr;
    m_cursorNode = nullptr;
    m_layer = nullptr;
    m_geometry.clear();
    m_geometryValid = false;
}

void GuideController::ensureNodes() {
    if (!m_layer || !m_layer->m_objectLayer) return;
    if (m_bandNode) return;

    m_bandNode = GuideNode::create();
    m_cursorNode = GuideNode::create();
    if (!m_bandNode || !m_cursorNode) {
        m_bandNode = nullptr;
        m_cursorNode = nullptr;
        return;
    }

    m_layer->m_objectLayer->addChild(m_bandNode, kBandZOrder);
    m_layer->m_objectLayer->addChild(m_cursorNode, kCursorZOrder);
    m_labels.attachTo(m_layer->m_objectLayer, kLabelZOrder);
}

void GuideController::onLevelReset() {
    m_anchorLatched = false;
    m_anchorX = 0.f;
    m_alignment.reset();
    m_startOffset.reset();
    publishAlignmentReport();
    m_predictor.clear();
    m_predictorPlayer2.clear();
    m_heights.clearSmoothing();

    Runtime::get().clearPresses();
    Runtime::get().clearJudgements();
    m_rhythmLane.reset();
    AssistState::get().buffer().resetConsumed();

    m_geometryValid = false;
}

void GuideController::onSettingsChanged() {
    m_geometryValid = false;
}

void GuideController::invalidateDerivedState() {
    m_heights.clearSmoothing();
    m_predictor.clear();
    m_predictorPlayer2.clear();
}

void GuideController::latchSpawnAnchor() {
    if (m_anchorLatched || !m_layer || !m_layer->m_player1) return;
    if (m_layer->m_attemptTime > static_cast<double>(kSpawnLatchSeconds)) {
        m_anchorLatched = true;
        m_anchorX = 0.f;
        return;
    }

    m_anchorX = m_layer->m_player1->getPositionX();
    m_anchorLatched = true;
}

void GuideController::rebuildGeometry() {
    m_geometry.clear();
    m_geometryValid = true;

    auto data = MacroStore::get().data();
    if (!m_layer) return;
    if (!data) {
        rebuildRhythmTimeline();
        return;
    }

    PlayLayer* layer = m_layer;
    double tickRate = effectiveTickRate();

    PositionResolver resolver = [layer, tickRate](double frame) -> float {
        if (frame < 0.0) frame = 0.0;
        return timeToLevelX(layer, static_cast<float>(frame / tickRate));
    };

    m_geometry = buildGeometry(*data, m_alignment.offsetFrames(), m_anchorX, resolver,
                               settings().showReleases);
    rebuildRhythmTimeline();
    invalidateDerivedState();
}

void GuideController::rebuildFromLevelScan() {
    if (!m_layer) return;

    if (!m_levelScanned) {
        m_scannedLevel = scanLevelObstacles(m_layer);
        m_levelScanned = true;
        log::info("{} Scan: {} obstacles, {} portals, {} speed changes", kLogTag,
                  m_scannedLevel.obstacles.size(), m_scannedLevel.modeChanges.size(),
                  m_scannedLevel.speedChanges.size());
    }

    if (!m_scannedLevel.valid || m_scannedLevel.obstacles.empty()) {
        m_rhythmLane.rebuild({});
        return;
    }

    auto const& config = settings();

    GenerationInput input;
    input.startSpeed = config.generateSpeed;
    input.startX = m_anchorX;
    input.startY = m_layer->m_player1 ? m_layer->m_player1->getPositionY() : 0.f;
    input.startMode = m_layer->m_player1 ? gamemodeOf(m_layer->m_player1) : Gamemode::Cube;
    input.horizonX = m_scannedLevel.obstacles.back().rightX + 200.f;

    auto notes = generateNotes(m_scannedLevel, input);

    std::vector<double> times;
    std::vector<double> holds;
    times.reserve(notes.size());
    holds.reserve(notes.size());

    for (auto const& note : notes) {
        float speedHere = speedAt(m_scannedLevel, note.levelX, input.startSpeed);
        double distance = static_cast<double>(note.levelX - input.startX);
        times.push_back(distance / static_cast<double>(std::max(speedHere, 1.f)));
        holds.push_back(static_cast<double>(note.holdSeconds));
    }

    log::info("{} Generated {} notes across all gamemodes", kLogTag, times.size());
    m_rhythmLane.rebuild(timelineFromHolds(times, holds));
}

void GuideController::rebuildRhythmTimeline() {
    if (settings().generateFromLevel) {
        rebuildFromLevelScan();
        publishAssistTargets();
        return;
    }

    auto data = MacroStore::get().data();
    if (!data) {
        m_rhythmLane.rebuild({});
        publishAssistTargets();
        return;
    }

    double tickRate = effectiveTickRate();
    double offsetSeconds = m_alignment.offsetFrames() / tickRate;
    m_rhythmLane.rebuild(buildRhythmTimeline(*data, offsetSeconds));
    publishAssistTargets();
}

void GuideController::publishAssistTargets() {
    AssistState::get().buffer().setTargets(m_rhythmLane.hitTimesForPlayerOne());
}

bool GuideController::worldGuideVisible() const {
    auto const& config = settings();
    if (!config.rhythmMode) return true;
    return !config.rhythmHideWorldGuide;
}

void GuideController::publishAlignmentReport() const {
    AlignmentReport report;
    report.locked = m_alignment.locked();
    report.searching = settings().autoAlign && !m_alignment.locked();
    report.offsetFrames = m_alignment.offsetFrames();
    report.presses = m_alignment.pressCount();
    report.support = m_alignment.supportCount();
    Runtime::get().setAlignmentReport(report);
}

double GuideController::effectiveTickRate() const {
    auto data = MacroStore::get().data();
    double tickRate = (data && data->framerate > 0.0) ? data->framerate : kDefaultTickRate;
    return tickRate * m_alignment.rateScale();
}

double GuideController::levelTimeNow() const {
    if (!m_layer) return 0.0;
    return m_layer->m_attemptTime + static_cast<double>(m_startOffset.seconds());
}

double GuideController::currentMacroFrame() const {
    if (!m_layer) return 0.0;
    return levelTimeNow() * effectiveTickRate();
}

bool GuideController::gamemodeAllowed() const {
    if (!m_layer || !m_layer->m_player1) return false;
    return settings().enabledFor(gamemodeOf(m_layer->m_player1));
}

void GuideController::consumePresses() {
    auto presses = Runtime::get().drainPresses();
    if (presses.empty() || m_geometry.markers.empty()) return;

    auto data = MacroStore::get().data();
    if (!data) return;

    auto const& config = settings();
    double tickRate = effectiveTickRate();
    double playerFrame = currentMacroFrame();
    double window = m_alignment.windowSeconds() * tickRate;

    bool searching = config.autoAlign && !m_alignment.locked();

    for (auto const& press : presses) {
        bool matched = false;
        double bestDelta = 0.0;
        std::vector<double> candidates;

        for (auto const& input : data->inputs) {
            if (!input.down) continue;
            if (input.player2 != press.player2) continue;

            double macroFrame = static_cast<double>(input.frame) + m_alignment.offsetFrames();
            double delta = playerFrame - macroFrame;
            if (std::fabs(delta) > window) continue;

            if (searching) candidates.push_back(delta);

            if (!matched || std::fabs(delta) < std::fabs(bestDelta)) {
                matched = true;
                bestDelta = delta;
            }
        }

        if (searching) {
            m_alignment.addCandidates(candidates);
            publishAlignmentReport();
            if (m_alignment.locked()) {
                log::info("{} Alignment locked: macro offset {:.0f} frames ({:.2f}s) from {} votes",
                          kLogTag, m_alignment.offsetFrames(),
                          m_alignment.offsetFrames() / tickRate, m_alignment.supportCount());
                m_geometryValid = false;
                searching = false;
            }
        }

        if (!matched) continue;

        if (!searching) {
            m_alignment.observeResidual(bestDelta, levelTimeNow());
            if (m_alignment.driftDetected()) {
                log::info("{} Timing drift detected, remeasuring (rate scale {:.4f})", kLogTag,
                          m_alignment.rateScale());
                m_geometryValid = false;
                publishAlignmentReport();
            }
        }

        m_rhythmLane.consumeNearestNote(levelTimeNow(), press.player2,
                                        static_cast<int>(std::lround(bestDelta)));
        m_rhythmLane.registerHit();

        if (!config.showAccuracy) continue;
        if (searching && !m_alignment.provisionalTrustworthy()) continue;

        PlayerObject* player = press.player2 ? m_layer->m_player2 : m_layer->m_player1;
        if (!player) continue;

        Judgement judgement;
        judgement.levelX = player->getPositionX();
        judgement.levelY = player->getPositionY();
        judgement.deltaFrames = static_cast<int>(std::lround(bestDelta));
        judgement.provisional = searching;
        Runtime::get().addJudgement(judgement);
    }
}

void GuideController::updateJudgements(float deltaSeconds) {
    Runtime::get().ageJudgements(deltaSeconds);
}

void GuideController::updatePredictions() {
    if (!settings().predictHeights || !m_layer) {
        m_predictor.clear();
        m_predictorPlayer2.clear();
        return;
    }

    if (auto* player = m_layer->m_player1) {
        Gamemode mode = gamemodeOf(player);
        float x = player->getPositionX();
        float y = player->getPositionY();
        if (m_predictor.needsRefresh(x, y, mode)) {
            m_predictor.rebuild(x, y, playerSpeedX(player), player->m_yVelocity, mode);
        }
    }

    if (auto* player2 = m_layer->m_player2; player2 && m_layer->m_gameState.m_isDualMode) {
        Gamemode mode = gamemodeOf(player2);
        float x = player2->getPositionX();
        float y = player2->getPositionY();
        if (m_predictorPlayer2.needsRefresh(x, y, mode)) {
            m_predictorPlayer2.rebuild(x, y, playerSpeedX(player2), player2->m_yVelocity, mode);
        }
    } else {
        m_predictorPlayer2.clear();
    }
}

void GuideController::observeHeights() {
    if (!settings().learnTrajectory || !m_layer || !m_layer->m_player1) return;
    auto* player = m_layer->m_player1;
    m_heights.observe(player->getPositionX(), player->getPositionY());
}

Viewport GuideController::computeViewport() const {
    Viewport view;
    if (!m_layer || !m_layer->m_player1) return view;

    auto* player = m_layer->m_player1;
    auto const& config = settings();

    float playerX = player->getPositionX();
    float playerY = player->getPositionY();
    CCSize windowSize = CCDirector::sharedDirector()->getWinSize();
    float scale = m_layer->m_objectLayer ? m_layer->m_objectLayer->getScale() : 1.f;
    if (scale <= 0.f) scale = 1.f;

    float halfSpan = windowSize.width / scale + kViewportSlack + config.cullPadding;

    float lead = config.lineLeadUnits + config.lineLeadSeconds * playerSpeedX(player);

    view.playerX = playerX;
    view.lineX = playerX + lead;
    view.clipX = config.clipBandsAtLine ? view.lineX : playerX;
    view.minX = playerX - kViewportSlack;
    view.maxX = playerX + halfSpan + lead;
    view.bandBottom = playerY - kBandVerticalMargin;
    view.bandTop = playerY + kBandVerticalMargin;
    view.markerY = playerY + (config.markerRow - 0.5f) * kMarkerRowSpan;
    view.markerYPlayer2 = view.markerY + kPlayerTwoMarkerOffset;

    return view;
}

float GuideController::resolveMarkerY(Marker const& marker, Viewport const& view) {
    float target = marker.player2 ? view.markerYPlayer2 : view.markerY;

    if (marker.hasRecordedY) {
        target = marker.y;
    } else {
        auto const& predictor = marker.player2 ? m_predictorPlayer2 : m_predictor;
        if (auto predicted = predictor.heightAt(marker.x)) {
            target = *predicted;
        } else if (settings().learnTrajectory) {
            if (auto observed = m_heights.lookup(marker.x)) {
                target = *observed;
            }
        }
    }

    return m_heights.smooth(marker.inputIndex, target);
}

void GuideController::drawBands(Viewport const& view) {
    if (!m_bandNode) return;

    auto const& config = settings();
    auto lower = std::lower_bound(m_geometry.bands.begin(), m_geometry.bands.end(), view.minX,
                                  [](Band const& band, float value) { return band.x1 < value; });

    for (auto it = lower; it != m_geometry.bands.end(); ++it) {
        if (it->x0 > view.maxX) break;
        GuidePainter::paintBand(m_bandNode, *it, view, config);
    }
}

void GuideController::drawMarkers(Viewport const& view) {
    if (!m_bandNode || !settings().showClicks) return;

    auto const& config = settings();
    auto lower = std::lower_bound(m_geometry.markers.begin(), m_geometry.markers.end(), view.minX,
                                  [](Marker const& marker, float value) { return marker.x < value; });

    for (auto it = lower; it != m_geometry.markers.end(); ++it) {
        if (it->x > view.maxX) break;
        GuidePainter::paintMarker(m_bandNode, *it, resolveMarkerY(*it, view), config);
    }
}

void GuideController::drawLabels() {
    m_labels.beginFrame();
    for (auto const& judgement : Runtime::get().judgements()) {
        m_labels.place(judgement.levelX, judgement.levelY,
                       judgement.deltaFrames,
                       judgement.remainingSeconds / kJudgementLifetimeSeconds,
                       judgement.provisional);
    }
    m_labels.endFrame();
}

void GuideController::redraw() {
    if (!m_bandNode || !m_cursorNode) return;

    m_bandNode->clear();
    m_cursorNode->clear();

    auto const& config = settings();
    if (!config.showGuide || !gamemodeAllowed() || !worldGuideVisible()) {
        m_labels.beginFrame();
        m_labels.endFrame();
        return;
    }

    Viewport view = computeViewport();

    drawBands(view);
    drawMarkers(view);

    if (config.showPath) {
        GuidePainter::paintTrajectory(m_cursorNode, m_predictor.samples(), config, false);
        GuidePainter::paintTrajectory(m_cursorNode, m_predictorPlayer2.samples(), config, true);
    }

    if (config.showPlayerLine) {
        GuidePainter::paintPlayerLine(m_cursorNode, view, config);
    }

    drawLabels();
}

void GuideController::onPostUpdate(float deltaSeconds) {
    if (!m_layer) return;
    ensureNodes();

    uint64_t revision = MacroStore::get().revision();
    if (revision != m_seenRevision) {
        m_seenRevision = revision;
        m_geometryValid = false;
    }

    m_startOffset.measure(m_layer);
    latchSpawnAnchor();

    if (!m_geometryValid) rebuildGeometry();

    observeHeights();
    consumePresses();

    if (!m_geometryValid) rebuildGeometry();

    updatePredictions();
    updateJudgements(deltaSeconds);
    redraw();

    AssistState::get().setSongTime(levelTimeNow());
    m_rhythmLane.update(levelTimeNow(), deltaSeconds);
}

} // namespace cgv
