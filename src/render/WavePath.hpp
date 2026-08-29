#pragma once

#include "model/ReplayData.hpp"
#include "render/PhysicsPredictor.hpp"

#include <functional>
#include <vector>

namespace cgv {

constexpr float kWaveSlopeSlow = 0.4f;
constexpr float kWaveSlopeNormal = 0.7f;
constexpr float kWaveSlopeFast = 1.0f;
constexpr float kWaveSlopeFaster = 1.3f;
constexpr float kWaveSlopeFastest = 1.6f;
constexpr float kMiniWaveSlopeScale = 2.0f;
constexpr size_t kMaxWavePathVertices = 512;
constexpr float kMinimumWaveSpeedX = 1.f;

using TimeToX = std::function<float(double seconds)>;

struct WavePathRequest {
    float originX = 0.f;
    float originY = 0.f;
    float speedX = 0.f;
    float slopeScale = 1.f;
    float gravitySign = 1.f;
    double startSeconds = 0.0;
    float limitX = 0.f;
    bool heldAtStart = false;
    bool player2 = false;
};

float waveSlopeForSpeed(float speedX);

std::vector<TrajectorySample> buildWavePath(ReplayData const& replay,
                                            WavePathRequest const& request,
                                            TimeToX const& timeToX);

} // namespace cgv
