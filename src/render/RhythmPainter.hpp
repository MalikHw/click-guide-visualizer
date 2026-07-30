#pragma once

#include "settings/Settings.hpp"

#include <cocos2d.h>

namespace cgv {

struct LaneLayout {
    float hitX = 0.f;
    float laneLeft = 0.f;
    float laneRight = 0.f;
    float centerY = 0.f;
    float halfHeight = 0.f;
    float playerTwoOffset = 0.f;
    float noteHalfWidth = 0.f;
    float noteHalfHeight = 0.f;
};

LaneLayout computeLaneLayout(cocos2d::CCSize const& windowSize, Snapshot const& config);

class RhythmPainter {
public:
    static void paintLaneBackground(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config);
    static void paintNote(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                          float startX, float endX, bool isHold, bool player2, float alphaScale);
    static void paintHitLine(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                             float flash);
    static void paintTimingWindow(cocos2d::CCDrawNode* node, LaneLayout const& layout, Snapshot const& config,
                                  float halfWidthPixels);
};

} // namespace cgv
