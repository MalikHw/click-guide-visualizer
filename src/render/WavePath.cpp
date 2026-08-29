#include "WavePath.hpp"

#include <algorithm>
#include <cmath>

namespace cgv {

namespace {

constexpr float kSpeedSlow = 251.16f;
constexpr float kSpeedNormal = 311.58f;
constexpr float kSpeedFast = 387.42f;
constexpr float kSpeedFaster = 468.f;
constexpr float kSpeedMidpointTolerance = 0.5f;

struct WaveStep {
    double seconds;
    bool down;
};

bool ownedByPath(ReplayInput const& input, bool player2) {
    return input.player2 == player2;
}

std::vector<WaveStep> collectSteps(ReplayData const& replay, WavePathRequest const& request,
                                   bool& heldAtStart) {
    std::vector<WaveStep> steps;
    heldAtStart = request.heldAtStart;

    double tickRate = replay.framerate > 0.0 ? replay.framerate : 240.0;

    steps.reserve(replay.inputs.size());
    for (auto const& input : replay.inputs) {
        if (!ownedByPath(input, request.player2)) continue;

        double seconds = static_cast<double>(input.frame) / tickRate;
        if (!std::isfinite(seconds)) continue;

        if (seconds <= request.startSeconds) {
            heldAtStart = input.down;
            continue;
        }

        steps.push_back(WaveStep{seconds, input.down});
    }

    std::stable_sort(steps.begin(), steps.end(),
                     [](WaveStep const& left, WaveStep const& right) {
                         return left.seconds < right.seconds;
                     });

    return steps;
}

void appendVertex(std::vector<TrajectorySample>& out, float x, float y) {
    if (!std::isfinite(x) || !std::isfinite(y)) return;
    out.push_back(TrajectorySample{x, y});
}

} // namespace

float waveSlopeForSpeed(float speedX) {
    float speed = std::fabs(speedX);
    if (speed < kSpeedSlow + kSpeedMidpointTolerance) return kWaveSlopeSlow;
    if (speed < kSpeedNormal + kSpeedMidpointTolerance) return kWaveSlopeNormal;
    if (speed < kSpeedFast + kSpeedMidpointTolerance) return kWaveSlopeFast;
    if (speed < kSpeedFaster + kSpeedMidpointTolerance) return kWaveSlopeFaster;
    return kWaveSlopeFastest;
}

std::vector<TrajectorySample> buildWavePath(ReplayData const& replay,
                                            WavePathRequest const& request,
                                            TimeToX const& timeToX) {
    std::vector<TrajectorySample> path;

    if (!std::isfinite(request.originX) || !std::isfinite(request.originY)) return path;
    if (!std::isfinite(request.limitX) || request.limitX <= request.originX) return path;

    float speedX = std::max(std::fabs(request.speedX), kMinimumWaveSpeedX);
    float scale = std::isfinite(request.slopeScale) && request.slopeScale > 0.f
                      ? request.slopeScale
                      : 1.f;
    float gravity = request.gravitySign < 0.f ? -1.f : 1.f;
    float verticalSpeed = speedX * waveSlopeForSpeed(speedX) * scale * gravity;

    bool held = false;
    std::vector<WaveStep> steps = collectSteps(replay, request, held);

    float x = request.originX;
    float y = request.originY;
    double seconds = request.startSeconds;

    appendVertex(path, x, y);

    for (auto const& step : steps) {
        if (path.size() >= kMaxWavePathVertices) break;
        if (x >= request.limitX) break;

        double span = step.seconds - seconds;
        if (span <= 0.0) {
            held = step.down;
            continue;
        }

        float nextX = timeToX ? timeToX(step.seconds) : x + speedX * static_cast<float>(span);
        if (!std::isfinite(nextX) || nextX <= x) {
            held = step.down;
            seconds = step.seconds;
            continue;
        }

        float travelled = nextX - x;
        float rise = (held ? verticalSpeed : -verticalSpeed) * (travelled / speedX);

        if (nextX >= request.limitX) {
            float clipped = request.limitX - x;
            y += rise * (clipped / travelled);
            x = request.limitX;
            appendVertex(path, x, y);
            return path;
        }

        x = nextX;
        y += rise;
        seconds = step.seconds;
        held = step.down;
        appendVertex(path, x, y);
    }

    if (path.size() < kMaxWavePathVertices && x < request.limitX) {
        float travelled = request.limitX - x;
        float rise = (held ? verticalSpeed : -verticalSpeed) * (travelled / speedX);
        appendVertex(path, request.limitX, y + rise);
    }

    return path;
}

} // namespace cgv
