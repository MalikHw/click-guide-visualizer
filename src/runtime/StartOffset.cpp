#include "StartOffset.hpp"

#include "core/Compat.hpp"
#include "render/GuideGeometry.hpp"

#include <Geode/Geode.hpp>
#include <cmath>

using namespace geode::prelude;

namespace cgv {

namespace {

float levelStartX(PlayLayer* layer) {
    return timeToLevelX(layer, 0.f);
}

} // namespace

float measureStartOffsetSeconds(PlayLayer* layer) {
    if (!layer || !layer->m_player1) return 0.f;

    float spawnX = layer->m_player1->getPositionX();
    float originX = levelStartX(layer);

    if (!std::isfinite(spawnX) || !std::isfinite(originX)) return 0.f;
    if (spawnX - originX < kStartPosDetectUnits) return 0.f;

    float seconds = invertLevelX(layer, spawnX);
    if (!std::isfinite(seconds) || seconds < 0.f) return 0.f;
    if (seconds > kMaxStartOffsetSeconds) return 0.f;

    return seconds;
}

void StartOffset::reset() {
    m_seconds = 0.f;
    m_settled = false;
}

void StartOffset::measure(PlayLayer* layer) {
    if (m_settled) return;
    if (!layer || !layer->m_player1) return;

    m_seconds = measureStartOffsetSeconds(layer);
    m_settled = true;

    if (m_seconds > 0.f) {
        log::info("{} Started {:.2f}s into the level, shifting the guide to match",
                  kLogTag, m_seconds);
    }
}

} // namespace cgv
