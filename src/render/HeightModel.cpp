#include "HeightModel.hpp"

#include "render/Easing.hpp"

#include <cmath>

namespace cgv {

int HeightModel::bucketOf(float levelX) {
    return static_cast<int>(std::floor(levelX / kHeightBucketSize));
}

void HeightModel::observe(float levelX, float levelY) {
    if (!std::isfinite(levelX) || !std::isfinite(levelY)) return;
    m_observed[bucketOf(levelX)] = levelY;
}

std::optional<float> HeightModel::lookup(float levelX) const {
    if (!std::isfinite(levelX)) return std::nullopt;

    int centre = bucketOf(levelX);
    for (int radius = 0; radius <= kHeightBucketProbeRadius; ++radius) {
        if (auto found = m_observed.find(centre - radius); found != m_observed.end()) {
            return found->second;
        }
        if (radius == 0) continue;
        if (auto found = m_observed.find(centre + radius); found != m_observed.end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

float HeightModel::smooth(size_t inputIndex, float target, float deltaSeconds) {
    if (!std::isfinite(target)) return target;

    auto existing = m_smoothed.find(inputIndex);
    if (existing == m_smoothed.end()) {
        if (m_smoothed.size() >= kMaxSmoothedEntries) m_smoothed.clear();
        m_smoothed[inputIndex] = target;
        return target;
    }

    float previous = existing->second;
    float next = std::fabs(target - previous) > kHeightSnapThreshold
        ? target
        : smoothTowards(previous, target, kHeightHalfLife, deltaSeconds);

    existing->second = next;
    return next;
}

void HeightModel::clearSmoothing() {
    m_smoothed.clear();
}

void HeightModel::clearObservations() {
    m_observed.clear();
}

void HeightModel::clearAll() {
    clearSmoothing();
    clearObservations();
}

} // namespace cgv
