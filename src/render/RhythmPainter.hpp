#pragma once

#include "settings/Settings.hpp"

#include <cocos2d.h>

namespace cgv {

struct LaneLayout {
    float hitY = 0.f;
    float laneTop = 0.f;
    float laneBottom = 0.f;
    float centerX = 0.f;
    float halfWidth = 0.f;
    float playerTwoInset = 0.f;
    float noteHalfWidth = 0.f;
    float noteHalfHeight = 0.f;
};

LaneLayout computeLaneLayout(cocos2d::CCSize const& windowSize, Snapshot const& config);
float noteHalfThicknessFor(LaneLayout const& layout, double gapSeconds, float pixelsPerSecond);

class RhythmPainter {
public:
    static void paintLaneBackground(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config);
    static void paintNote(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                          float hitLineY, float releaseY, bool isHold, bool player2, float alphaScale,
                          float halfThickness);
    static void paintHoldBody(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                              float hitLineY, float releaseY, bool player2, float alphaScale,
                              float halfThickness);
    static void paintNoteHead(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                              float hitLineY, bool player2, float alphaScale, float halfThickness);
    static void paintHitLine(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                             float flash);
    static void paintTimingWindow(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                                  float halfHeightPixels);
};

} // namespace cgv
