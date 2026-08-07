#pragma once

namespace cgv {

constexpr float kMaxSaneDelta = 0.1f;

float easeOutCubic(float progress);
float easeOutQuad(float progress);
float easeInOutCubic(float progress);

float smoothTowards(float current, float target, float halfLifeSeconds, float deltaSeconds);
float approachRate(float halfLifeSeconds, float deltaSeconds);
float clampDelta(float deltaSeconds);

} // namespace cgv
