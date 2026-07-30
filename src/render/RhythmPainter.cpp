#include "RhythmPainter.hpp"

#include "render/GuidePainter.hpp"

#include <algorithm>

using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kLaneBorderAlphaScale = 0.55f;
constexpr float kHitLineWidth = 3.f;
constexpr float kHitLineOverhang = 8.f;
constexpr float kFlashWidthGain = 5.f;
constexpr float kFlashAlphaGain = 0.55f;
constexpr float kHoldBodyScale = 0.5f;
constexpr float kNoteHeightScale = 1.6f;
constexpr float kNoteHeadBrighten = 0.25f;
constexpr float kWindowFillAlpha = 0.18f;
constexpr ccColor3B kWindowColor{120, 220, 255};

void fillRect(CCDrawNode* node, float left, float bottom, float right, float top,
              ccColor4F fill, float borderWidth, ccColor4F border) {
    if (right <= left || top <= bottom) return;
    CCPoint quad[4] = {
        ccp(left, bottom),
        ccp(right, bottom),
        ccp(right, top),
        ccp(left, top),
    };
    node->drawPolygon(quad, 4, fill, borderWidth, border);
}

} // namespace

LaneLayout computeLaneLayout(CCSize const& windowSize, Snapshot const& config) {
    LaneLayout layout;

    layout.halfHeight = std::max(config.rhythmLaneHeight, 10.f) * 0.5f;
    layout.centerY = windowSize.height * std::clamp(config.rhythmLaneY, 0.f, 1.f);
    layout.centerY = std::clamp(layout.centerY, layout.halfHeight, windowSize.height - layout.halfHeight);

    layout.laneLeft = 0.f;
    layout.laneRight = windowSize.width;
    layout.hitX = windowSize.width * std::clamp(config.rhythmHitX, 0.05f, 0.95f);
    layout.playerTwoOffset = layout.halfHeight * 0.5f;

    layout.noteHalfWidth = std::max(config.rhythmNoteWidth, 1.f) * 0.5f;
    layout.noteHalfHeight = std::min(layout.noteHalfWidth * kNoteHeightScale, layout.halfHeight * 0.8f);

    return layout;
}

void RhythmPainter::paintLaneBackground(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config) {
    ccColor3B base = config.lineColor;
    float alpha = config.rhythmLaneOpacity;

    fillRect(node, layout.laneLeft, layout.centerY - layout.halfHeight,
             layout.laneRight, layout.centerY + layout.halfHeight,
             withAlpha(ccColor3B{10, 12, 18}, alpha),
             config.borderWidth,
             withAlpha(base, alpha * kLaneBorderAlphaScale));
}

void RhythmPainter::paintTimingWindow(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                                      float halfWidthPixels) {
    if (halfWidthPixels <= 0.f) return;

    fillRect(node, layout.hitX, layout.centerY - layout.halfHeight,
             layout.hitX + halfWidthPixels, layout.centerY + layout.halfHeight,
             withAlpha(kWindowColor, kWindowFillAlpha), 0.f, withAlpha(kWindowColor, 0.f));
}

void RhythmPainter::paintNote(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                              float startX, float endX, bool isHold, bool player2, float alphaScale) {
    ccColor3B base = player2 ? kPlayerTwoColor : config.lineColor;
    float centre = layout.centerY + (player2 ? layout.playerTwoOffset : 0.f);

    float fillAlpha = config.indicatorOpacity * config.fillOpacity * alphaScale;
    float borderAlpha = config.indicatorOpacity * config.borderOpacity * alphaScale;

    if (isHold) {
        float bodyHalf = layout.noteHalfHeight * kHoldBodyScale;
        float left = std::max(startX, layout.laneLeft);
        float right = std::min(endX, layout.laneRight);
        fillRect(node, left, centre - bodyHalf, right, centre + bodyHalf,
                 withAlpha(base, fillAlpha), config.borderWidth, withAlpha(base, borderAlpha));
    }

    fillRect(node, startX - layout.noteHalfWidth, centre - layout.noteHalfHeight,
             startX + layout.noteHalfWidth, centre + layout.noteHalfHeight,
             withAlpha(base, fillAlpha + config.fillOpacity * alphaScale * kNoteHeadBrighten),
             config.borderWidth,
             withAlpha(kAccentWhite, borderAlpha));
}

void RhythmPainter::paintHitLine(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                                 float flash) {
    float clamped = std::clamp(flash, 0.f, 1.f);
    float halfWidth = (kHitLineWidth + clamped * kFlashWidthGain) * 0.5f;
    float alpha = std::clamp(config.lineOpacity + clamped * kFlashAlphaGain, 0.f, 1.f);

    fillRect(node, layout.hitX - halfWidth, layout.centerY - layout.halfHeight - kHitLineOverhang,
             layout.hitX + halfWidth, layout.centerY + layout.halfHeight + kHitLineOverhang,
             withAlpha(kAccentWhite, alpha), 0.f, withAlpha(kAccentWhite, 0.f));
}

} // namespace cgv
