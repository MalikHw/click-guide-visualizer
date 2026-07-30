#include "PhysicsPredictor.hpp"

#include <algorithm>
#include <cmath>

namespace cgv {

namespace {

constexpr ModePhysics kCubePhysics{2900.f, 620.f, 0.f, 1000.f, false, false};
constexpr ModePhysics kShipPhysics{2400.f, 0.f, 4200.f, 900.f, true, false};
constexpr ModePhysics kBallPhysics{2900.f, 0.f, 0.f, 1000.f, false, false};
constexpr ModePhysics kUfoPhysics{2800.f, 480.f, 0.f, 900.f, false, false};
constexpr ModePhysics kWavePhysics{0.f, 0.f, 0.f, 0.f, true, true};
constexpr ModePhysics kRobotPhysics{2900.f, 600.f, 0.f, 1000.f, false, false};
constexpr ModePhysics kSpiderPhysics{2900.f, 0.f, 0.f, 1000.f, false, false};
constexpr ModePhysics kSwingPhysics{2400.f, 0.f, 0.f, 900.f, true, true};

constexpr float kMinimumSpeedX = 1.f;

} // namespace

ModePhysics physicsFor(Gamemode mode) {
    switch (mode) {
        case Gamemode::Cube: return kCubePhysics;
        case Gamemode::Ship: return kShipPhysics;
        case Gamemode::Ball: return kBallPhysics;
        case Gamemode::Ufo: return kUfoPhysics;
        case Gamemode::Wave: return kWavePhysics;
        case Gamemode::Robot: return kRobotPhysics;
        case Gamemode::Spider: return kSpiderPhysics;
        case Gamemode::Swing: return kSwingPhysics;
    }
    return kCubePhysics;
}

bool PhysicsPredictor::needsRefresh(float playerX, float playerY, Gamemode mode) const {
    if (!m_valid) return true;
    if (mode != m_mode) return true;
    if (std::fabs(playerX - m_anchorX) > kPredictionRefreshX) return true;
    if (std::fabs(playerY - m_anchorY) > kPredictionRefreshY) return true;
    return false;
}

void PhysicsPredictor::rebuild(float playerX, float playerY, float speedX, double yVelocity, Gamemode mode) {
    m_samples.clear();
    m_anchorX = playerX;
    m_anchorY = playerY;
    m_mode = mode;
    m_valid = true;

    if (!std::isfinite(playerX) || !std::isfinite(playerY)) {
        m_valid = false;
        return;
    }

    ModePhysics physics = physicsFor(mode);
    float horizontalSpeed = std::max(speedX, kMinimumSpeedX);
    float step = 1.f / kPredictionStepRate;
    int stepCount = static_cast<int>(kPredictionHorizonSeconds * kPredictionStepRate);

    m_samples.reserve(static_cast<size_t>(stepCount) + 1);

    float x = playerX;
    float y = playerY;
    float verticalSpeed = static_cast<float>(yVelocity);
    if (!std::isfinite(verticalSpeed)) verticalSpeed = 0.f;

    m_samples.push_back(TrajectorySample{x, y});

    for (int index = 0; index < stepCount; ++index) {
        if (!physics.slopeBased) {
            verticalSpeed -= physics.gravity * step;
            if (physics.maxFallSpeed > 0.f) {
                verticalSpeed = std::clamp(verticalSpeed, -physics.maxFallSpeed, physics.maxFallSpeed);
            }
            y += verticalSpeed * step;
        }

        x += horizontalSpeed * step;
        if (!std::isfinite(x) || !std::isfinite(y)) break;

        m_samples.push_back(TrajectorySample{x, y});
    }
}

void PhysicsPredictor::clear() {
    m_samples.clear();
    m_valid = false;
}

std::optional<float> PhysicsPredictor::heightAt(float levelX) const {
    if (m_samples.size() < 2 || !std::isfinite(levelX)) return std::nullopt;
    if (levelX < m_samples.front().x || levelX > m_samples.back().x) return std::nullopt;

    auto upper = std::lower_bound(m_samples.begin(), m_samples.end(), levelX,
                                  [](TrajectorySample const& sample, float value) { return sample.x < value; });

    if (upper == m_samples.begin()) return upper->y;
    if (upper == m_samples.end()) return m_samples.back().y;

    auto lower = std::prev(upper);
    float span = upper->x - lower->x;
    if (span <= 0.f) return lower->y;

    float ratio = (levelX - lower->x) / span;
    return lower->y + (upper->y - lower->y) * ratio;
}

} // namespace cgv
