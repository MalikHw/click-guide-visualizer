#pragma once

#include "core/Gamemode.hpp"

#include <optional>
#include <vector>

class PlayerObject;

namespace cgv {

constexpr float kPredictionStepRate = 120.f;
constexpr float kPredictionHorizonSeconds = 2.f;
constexpr float kPredictionRefreshX = 30.f;
constexpr float kPredictionRefreshY = 25.f;

struct ModePhysics {
    float gravity;
    float jumpImpulse;
    float holdAcceleration;
    float maxFallSpeed;
    bool flight;
    bool slopeBased;
};

ModePhysics physicsFor(Gamemode mode);

struct TrajectorySample {
    float x = 0.f;
    float y = 0.f;
};

class PhysicsPredictor {
public:
    bool needsRefresh(float playerX, float playerY, Gamemode mode) const;
    void rebuild(float playerX, float playerY, float speedX, double yVelocity, Gamemode mode);
    void clear();

    std::optional<float> heightAt(float levelX) const;
    std::vector<TrajectorySample> const& samples() const { return m_samples; }
    bool empty() const { return m_samples.empty(); }

private:
    std::vector<TrajectorySample> m_samples;
    float m_anchorX = 0.f;
    float m_anchorY = 0.f;
    Gamemode m_mode = Gamemode::Cube;
    bool m_valid = false;
};

} // namespace cgv
