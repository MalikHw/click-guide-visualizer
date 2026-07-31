#include "HitEffects.hpp"

#include "render/GuidePainter.hpp"

#include <algorithm>
#include <cmath>

using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kTwoPi = 6.28318531f;
constexpr float kRingThickness = 2.2f;
constexpr float kSpokeThickness = 1.8f;
constexpr float kFlashAlpha = 0.55f;

float easeOutCubic(float progress) {
    float inverted = 1.f - std::clamp(progress, 0.f, 1.f);
    return 1.f - inverted * inverted * inverted;
}

ccColor3B effectColor(HitEffect const& effect, Snapshot const& config) {
    if (effect.player2) return kPlayerTwoColor;
    return judgementColor(effect.deltaFrames);
}

void drawRing(CCDrawNode* node, float centreX, float centreY, float radius,
              ccColor4F color, float thickness) {
    if (radius <= 0.f) return;

    constexpr int kSegments = 20;
    CCPoint previous = ccp(centreX + radius, centreY);

    for (int step = 1; step <= kSegments; ++step) {
        float angle = kTwoPi * static_cast<float>(step) / static_cast<float>(kSegments);
        CCPoint current = ccp(centreX + std::cos(angle) * radius,
                              centreY + std::sin(angle) * radius);
        node->drawSegment(previous, current, thickness, color);
        previous = current;
    }
}

void drawBurst(CCDrawNode* node, float centreX, float centreY, float reach, ccColor4F color) {
    if (reach <= 0.f) return;

    int spokes = static_cast<int>(kBurstSpokeCount);
    for (int index = 0; index < spokes; ++index) {
        float angle = kTwoPi * static_cast<float>(index) / kBurstSpokeCount;
        float dirX = std::cos(angle);
        float dirY = std::sin(angle);
        CCPoint from = ccp(centreX + dirX * reach * 0.45f, centreY + dirY * reach * 0.45f);
        CCPoint to = ccp(centreX + dirX * reach, centreY + dirY * reach);
        node->drawSegment(from, to, kSpokeThickness, color);
    }
}

void drawShrinkingNote(CCDrawNode* node, LaneLayout const& layout, float centreX, float centreY,
                       float scale, ccColor4F color) {
    float halfWidth = layout.noteHalfWidth * scale;
    float halfHeight = layout.noteHalfHeight * scale;
    if (halfWidth <= 0.f || halfHeight <= 0.f) return;

    CCPoint quad[4] = {
        ccp(centreX - halfWidth, centreY - halfHeight),
        ccp(centreX + halfWidth, centreY - halfHeight),
        ccp(centreX + halfWidth, centreY + halfHeight),
        ccp(centreX - halfWidth, centreY + halfHeight),
    };
    node->drawPolygon(quad, 4, color, 0.f, ccc4f(0.f, 0.f, 0.f, 0.f));
}

} // namespace

void HitEffects::spawn(float screenY, bool player2, int deltaFrames, float lifetime) {
    if (!std::isfinite(screenY)) return;

    HitEffect effect;
    effect.screenY = screenY;
    effect.age = 0.f;
    effect.lifetime = std::max(lifetime, 0.05f);
    effect.player2 = player2;
    effect.deltaFrames = deltaFrames;
    effect.active = true;

    for (auto& slot : m_effects) {
        if (!slot.active) {
            slot = effect;
            return;
        }
    }

    if (m_effects.size() < kMaxHitEffects) {
        m_effects.push_back(effect);
        return;
    }

    m_effects.front() = effect;
}

void HitEffects::advance(float deltaSeconds) {
    for (auto& effect : m_effects) {
        if (!effect.active) continue;
        effect.age += deltaSeconds;
        if (effect.age >= effect.lifetime) effect.active = false;
    }
}

void HitEffects::clear() {
    for (auto& effect : m_effects) effect.active = false;
}

bool HitEffects::empty() const {
    for (auto const& effect : m_effects) {
        if (effect.active) return false;
    }
    return true;
}

void HitEffects::paint(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config) const {
    if (!node) return;

    for (auto const& effect : m_effects) {
        if (!effect.active) continue;

        float progress = std::clamp(effect.age / effect.lifetime, 0.f, 1.f);
        float eased = easeOutCubic(progress);
        float fade = 1.f - progress;
        if (fade <= 0.f) continue;

        float centreX = layout.centerX;
        float centreY = effect.screenY;
        float ringRadius = layout.halfWidth;
        ccColor3B base = effectColor(effect, config);
        float alpha = fade * config.indicatorOpacity;

        float ringScale = kRingStartScale + (kRingEndScale - kRingStartScale) * eased;
        drawRing(node, centreX, centreY, ringRadius * ringScale,
                 withAlpha(base, alpha), kRingThickness);

        drawBurst(node, centreX, centreY, kBurstSpokeLength * eased,
                  withAlpha(base, alpha * 0.8f));

        float shrink = std::max(1.f - eased, kShrinkFloor);
        drawShrinkingNote(node, layout, centreX, centreY, shrink,
                          withAlpha(kAccentWhite, alpha * kFlashAlpha));
    }
}

} // namespace cgv
