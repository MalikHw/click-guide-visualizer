#include "GuidePainter.hpp"

#include "runtime/Runtime.hpp"

#include <algorithm>
#include <cmath>

using namespace cocos2d;

namespace cgv {

namespace {

constexpr ccColor3B kPerfectColor{80, 255, 120};
constexpr ccColor3B kGoodColor{255, 220, 70};
constexpr ccColor3B kLateColor{255, 90, 90};
constexpr float kTrajectoryAlphaScale = 0.7f;
constexpr size_t kTrajectoryStride = 4;

ccColor3B colorForOwner(bool player2, Snapshot const& config) {
    return player2 ? kPlayerTwoColor : config.lineColor;
}

void drawVerticalLine(CCDrawNode* node, float x, float bottom, float top, float halfWidth, ccColor4F color) {
    CCPoint quad[4] = {
        ccp(x - halfWidth, bottom),
        ccp(x + halfWidth, bottom),
        ccp(x + halfWidth, top),
        ccp(x - halfWidth, top),
    };
    node->drawPolygon(quad, 4, color, 0.f, ccc4FFromccc3B(ccBLACK));
}

} // namespace

ccColor4F withAlpha(ccColor3B color, float alpha) {
    float clamped = std::clamp(alpha, 0.f, 1.f);
    return ccc4f(color.r / 255.f, color.g / 255.f, color.b / 255.f, clamped);
}

ccColor3B judgementColor(int deltaFrames) {
    int magnitude = std::abs(deltaFrames);
    if (magnitude <= kPerfectFrameWindow) return kPerfectColor;
    if (magnitude <= kGoodFrameWindow) return kGoodColor;
    return kLateColor;
}

void GuidePainter::paintBand(CCDrawNode* node, Band const& band, Viewport const& view, Snapshot const& config) {
    float left = std::max(band.x0, view.clipX);
    float right = band.x1;
    if (right <= left) return;

    ccColor3B base = colorForOwner(band.player2, config);
    float fillAlpha = config.bandOpacity * config.fillOpacity;
    float borderAlpha = config.bandOpacity * config.borderOpacity;

    CCPoint quad[4] = {
        ccp(left, view.bandBottom),
        ccp(right, view.bandBottom),
        ccp(right, view.bandTop),
        ccp(left, view.bandTop),
    };

    node->drawPolygon(quad, 4, withAlpha(base, fillAlpha), config.borderWidth, withAlpha(base, borderAlpha));

    if (config.showBandLines && band.pressX >= view.clipX) {
        drawVerticalLine(node, band.pressX, view.bandBottom, view.bandTop,
                         std::max(config.lineWidth, 0.05f) * 0.5f,
                         withAlpha(kAccentWhite, config.lineOpacity * config.bandOpacity));
    }
}

void GuidePainter::paintMarker(CCDrawNode* node, Marker const& marker, float resolvedY, Snapshot const& config) {
    float half = std::max(config.markerSize, 1.f) * 0.5f;
    ccColor3B base = marker.player2 ? kPlayerTwoColor : kAccentWhite;

    CCPoint quad[4] = {
        ccp(marker.x - half, resolvedY - half),
        ccp(marker.x + half, resolvedY - half),
        ccp(marker.x + half, resolvedY + half),
        ccp(marker.x - half, resolvedY + half),
    };

    float fillAlpha = config.indicatorOpacity * config.fillOpacity;
    float borderAlpha = config.indicatorOpacity * config.borderOpacity;

    node->drawPolygon(quad, 4, withAlpha(base, fillAlpha), config.borderWidth, withAlpha(base, borderAlpha));
}

void GuidePainter::paintPlayerLine(CCDrawNode* node, Viewport const& view, Snapshot const& config) {
    drawVerticalLine(node, view.lineX, view.bandBottom, view.bandTop,
                     std::max(config.lineThickness, 0.1f) * 0.5f,
                     withAlpha(kAccentWhite, config.lineOpacity * config.indicatorOpacity));
}

void GuidePainter::paintTrajectory(CCDrawNode* node, std::vector<TrajectorySample> const& samples,
                                   Snapshot const& config, bool player2) {
    if (samples.size() < 2) return;

    ccColor4F color = withAlpha(colorForOwner(player2, config),
                                config.indicatorOpacity * kTrajectoryAlphaScale);
    float thickness = std::max(config.lineWidth, 0.1f);

    for (size_t index = kTrajectoryStride; index < samples.size(); index += kTrajectoryStride) {
        auto const& from = samples[index - kTrajectoryStride];
        auto const& to = samples[index];
        node->drawSegment(ccp(from.x, from.y), ccp(to.x, to.y), thickness, color);
    }
}

} // namespace cgv
