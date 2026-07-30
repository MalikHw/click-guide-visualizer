#include "Framerate.hpp"

#include <array>
#include <cmath>

namespace cgv {

namespace {

constexpr std::array<double, 10> kCommonTickRates = {
    60.0, 120.0, 144.0, 240.0, 360.0, 480.0, 600.0, 720.0, 960.0, 1000.0
};

constexpr double kSnapTolerance = 0.06;
constexpr float kMinUsableDuration = 0.001f;

double snapToCommonRate(double derived) {
    for (double candidate : kCommonTickRates) {
        if (std::fabs(derived - candidate) <= candidate * kSnapTolerance) {
            return candidate;
        }
    }
    return derived;
}

} // namespace

bool isPlausibleTickRate(double rate) {
    if (!std::isfinite(rate)) return false;
    return rate >= kMinPlausibleTickRate && rate <= kMaxPlausibleTickRate;
}

double recoverTickRate(double declared, size_t frameSpan, float duration) {
    if (isPlausibleTickRate(declared)) return declared;
    if (frameSpan == 0 || duration <= kMinUsableDuration) return kDefaultTickRate;

    double derived = static_cast<double>(frameSpan) / static_cast<double>(duration);
    if (!isPlausibleTickRate(derived)) return kDefaultTickRate;

    return snapToCommonRate(derived);
}

} // namespace cgv
