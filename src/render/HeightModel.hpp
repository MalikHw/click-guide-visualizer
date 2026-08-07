#pragma once

#include <cstddef>
#include <optional>
#include <unordered_map>

namespace cgv {

constexpr float kHeightBucketSize = 10.f;
constexpr int kHeightBucketProbeRadius = 2;
constexpr float kHeightHalfLife = 0.06f;
constexpr float kHeightSnapThreshold = 120.f;

class HeightModel {
public:
    void observe(float levelX, float levelY);
    std::optional<float> lookup(float levelX) const;

    float smooth(size_t inputIndex, float target, float deltaSeconds);
    void clearSmoothing();
    void clearObservations();
    void clearAll();

private:
    static int bucketOf(float levelX);

    std::unordered_map<int, float> m_observed;
    std::unordered_map<size_t, float> m_smoothed;
};

} // namespace cgv
