#pragma once

#include "render/GuideGeometry.hpp"
#include "render/PhysicsPredictor.hpp"
#include "settings/Settings.hpp"

#include <cocos2d.h>
#include <vector>

namespace cgv {

constexpr cocos2d::ccColor3B kPlayerTwoColor{255, 150, 40};
constexpr cocos2d::ccColor3B kAccentWhite{255, 255, 255};

struct Viewport {
    float minX = 0.f;
    float maxX = 0.f;
    float playerX = 0.f;
    float lineX = 0.f;
    float clipX = 0.f;
    float bandBottom = 0.f;
    float bandTop = 0.f;
    float markerY = 0.f;
    float markerYPlayer2 = 0.f;
};

class GuidePainter {
public:
    static void paintBand(cocos2d::CCDrawNode* node, Band const& band, Viewport const& view, Snapshot const& config);
    static void paintMarker(cocos2d::CCDrawNode* node, Marker const& marker, float resolvedY, Snapshot const& config);
    static void paintPlayerLine(cocos2d::CCDrawNode* node, Viewport const& view, Snapshot const& config);
    static void paintTrajectory(cocos2d::CCDrawNode* node, std::vector<TrajectorySample> const& samples,
                                Snapshot const& config, bool player2);
};

cocos2d::ccColor4F withAlpha(cocos2d::ccColor3B color, float alpha);
cocos2d::ccColor3B judgementColor(int deltaFrames);

} // namespace cgv
