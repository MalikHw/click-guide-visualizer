#include "RhythmPainter.hpp"

#include "render/GuidePainter.hpp"

#include <algorithm>

using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kLaneBorderAlphaScale = 0.55f;
constexpr float kApproachTintAlphaScale = 0.12f;
constexpr float kHitLineThickness = 5.f;
constexpr float kHitLineOverhang = 6.f;
constexpr float kFlashThicknessGain = 7.f;
constexpr float kFlashAlphaGain = 0.55f;
constexpr float kHoldBodyInsetScale = 0.28f;
constexpr float kNoteEdgeInset = 3.f;
constexpr float kPlayerTwoInsetScale = 0.22f;
constexpr float kNoteHeadBrighten = 0.25f;
constexpr float kWindowFillAlpha = 0.18f;
constexpr ccColor3B kLaneBackdrop{10, 12, 18};
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

float noteLeftEdge(LaneLayout const& layout, bool player2) {
    float inset = kNoteEdgeInset + (player2 ? layout.playerTwoInset : 0.f);
    return layout.centerX - layout.halfWidth + inset;
}

float noteRightEdge(LaneLayout const& layout, bool player2) {
    float inset = kNoteEdgeInset + (player2 ? layout.playerTwoInset : 0.f);
    return layout.centerX + layout.halfWidth - inset;
}

} // namespace

LaneLayout computeLaneLayout(CCSize const& windowSize, Snapshot const& config) {
    LaneLayout layout;

    layout.halfWidth = std::max(config.rhythmLaneWidth, 40.f) * 0.5f;
    layout.centerX = windowSize.width * std::clamp(config.rhythmLaneX, 0.f, 1.f);
    layout.centerX = std::clamp(layout.centerX, layout.halfWidth, windowSize.width - layout.halfWidth);

    layout.laneBottom = 0.f;
    layout.laneTop = windowSize.height;
    layout.hitY = windowSize.height * std::clamp(config.rhythmHitY, 0.05f, 0.95f);

    layout.playerTwoInset = layout.halfWidth * kPlayerTwoInsetScale;

    layout.noteHalfWidth = std::max(layout.halfWidth - kNoteEdgeInset, 1.f);
    layout.noteHalfHeight = std::max(config.rhythmNoteThickness, 2.f) * 0.5f;

    return layout;
}

void RhythmPainter::paintLaneBackground(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config) {
    ccColor3B base = config.lineColor;
    float alpha = config.rhythmLaneOpacity;

    fillRect(node, layout.centerX - layout.halfWidth, layout.laneBottom,
             layout.centerX + layout.halfWidth, layout.laneTop,
             withAlpha(kLaneBackdrop, alpha),
             config.borderWidth,
             withAlpha(base, alpha * kLaneBorderAlphaScale));

    fillRect(node, layout.centerX - layout.halfWidth, layout.laneBottom,
             layout.centerX + layout.halfWidth, layout.hitY,
             withAlpha(base, alpha * kApproachTintAlphaScale),
             0.f, withAlpha(base, 0.f));
}

void RhythmPainter::paintTimingWindow(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                                      float halfHeightPixels) {
    if (halfHeightPixels <= 0.f) return;

    fillRect(node, layout.centerX - layout.halfWidth, layout.hitY,
             layout.centerX + layout.halfWidth, layout.hitY + halfHeightPixels,
             withAlpha(kWindowColor, kWindowFillAlpha), 0.f, withAlpha(kWindowColor, 0.f));
}

void RhythmPainter::paintNote(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                              float hitLineY, float releaseY, bool isHold, bool player2, float alphaScale) {
    ccColor3B base = player2 ? kPlayerTwoColor : config.lineColor;

    float left = noteLeftEdge(layout, player2);
    float right = noteRightEdge(layout, player2);

    float fillAlpha = config.indicatorOpacity * config.fillOpacity * alphaScale;
    float borderAlpha = config.indicatorOpacity * config.borderOpacity * alphaScale;

    if (isHold) {
        float bodyInset = (right - left) * kHoldBodyInsetScale * 0.5f;
        float bodyBottom = std::max(hitLineY, layout.laneBottom);
        float bodyTop = std::min(releaseY, layout.laneTop);
        fillRect(node, left + bodyInset, bodyBottom, right - bodyInset, bodyTop,
                 withAlpha(base, fillAlpha), config.borderWidth, withAlpha(base, borderAlpha));
    }

    fillRect(node, left, hitLineY - layout.noteHalfHeight, right, hitLineY + layout.noteHalfHeight,
             withAlpha(base, fillAlpha + config.fillOpacity * alphaScale * kNoteHeadBrighten),
             config.borderWidth,
             withAlpha(kAccentWhite, borderAlpha));
}

void RhythmPainter::paintHitLine(CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                                 float flash) {
    float clamped = std::clamp(flash, 0.f, 1.f);
    float halfThickness = (kHitLineThickness + clamped * kFlashThicknessGain) * 0.5f;
    float alpha = std::clamp(config.lineOpacity + clamped * kFlashAlphaGain, 0.f, 1.f);

    fillRect(node, layout.centerX - layout.halfWidth - kHitLineOverhang, layout.hitY - halfThickness,
             layout.centerX + layout.halfWidth + kHitLineOverhang, layout.hitY + halfThickness,
             withAlpha(kAccentWhite, alpha), 0.f, withAlpha(kAccentWhite, 0.f));
}

} // namespace cgv
