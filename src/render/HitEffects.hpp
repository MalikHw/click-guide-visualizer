#pragma once

#include "render/RhythmPainter.hpp"
#include "settings/Settings.hpp"

#include <cocos2d.h>
#include <cstddef>
#include <vector>

namespace cgv {

struct HitEffect {
    float screenY = 0.f;
    float age = 0.f;
    float lifetime = 0.25f;
    bool player2 = false;
    int deltaFrames = 0;
    bool active = false;
};

constexpr size_t kMaxHitEffects = 16;
constexpr float kRingStartScale = 1.f;
constexpr float kRingEndScale = 1.75f;
constexpr float kBurstSpokeCount = 6.f;
constexpr float kBurstSpokeLength = 30.f;
constexpr float kShrinkFloor = 0.15f;

class HitEffects {
public:
    void spawn(float screenY, bool player2, int deltaFrames, float lifetime);
    void advance(float deltaSeconds);
    void clear();

    void paint(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config) const;

    bool empty() const;

private:
    std::vector<HitEffect> m_effects;
};

} // namespace cgv
