#include "Easing.hpp"

#include <algorithm>
#include <cmath>

namespace cgv {

namespace {

constexpr float kLogHalf = -0.6931472f;

} // namespace

float clampDelta(float deltaSeconds) {
    if (std::isnan(deltaSeconds)) return 0.f;
    if (deltaSeconds <= 0.f) return 0.f;
    if (deltaSeconds > kMaxSaneDelta) return kMaxSaneDelta;
    return deltaSeconds;
}

float easeOutCubic(float progress) {
    float inverted = 1.f - std::clamp(progress, 0.f, 1.f);
    return 1.f - inverted * inverted * inverted;
}

float easeOutQuad(float progress) {
    float inverted = 1.f - std::clamp(progress, 0.f, 1.f);
    return 1.f - inverted * inverted;
}

float easeInOutCubic(float progress) {
    float t = std::clamp(progress, 0.f, 1.f);
    if (t < 0.5f) return 4.f * t * t * t;
    float shifted = -2.f * t + 2.f;
    return 1.f - shifted * shifted * shifted * 0.5f;
}

float approachRate(float halfLifeSeconds, float deltaSeconds) {
    float delta = clampDelta(deltaSeconds);
    if (delta <= 0.f) return 0.f;
    if (!std::isfinite(halfLifeSeconds) || halfLifeSeconds <= 0.f) return 1.f;

    return 1.f - std::exp(kLogHalf * delta / halfLifeSeconds);
}

float smoothTowards(float current, float target, float halfLifeSeconds, float deltaSeconds) {
    if (!std::isfinite(target)) return current;
    if (!std::isfinite(current)) return target;

    float rate = approachRate(halfLifeSeconds, deltaSeconds);
    return current + (target - current) * rate;
}

} // namespace cgv
