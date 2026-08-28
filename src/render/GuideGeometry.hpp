#pragma once

#include "model/ReplayData.hpp"

#include <cstddef>
#include <functional>
#include <vector>

class PlayLayer;

namespace cgv {

constexpr float kMinimumBandWidth = 1.5f;
constexpr int kBisectionExpansions = 12;
constexpr int kBisectionHalvings = 40;
constexpr float kBisectionInitialSpan = 1.f;

constexpr float kUnlimitedMarkerGap = 1.0e9f;

struct Band {
    float x0 = 0.f;
    float x1 = 0.f;
    float pressX = 0.f;
    bool player2 = false;
    size_t inputIndex = 0;
};

struct Marker {
    float x = 0.f;
    float y = 0.f;
    bool player2 = false;
    bool hasRecordedY = false;
    bool isRelease = false;
    size_t inputIndex = 0;
    float gapToNeighbour = kUnlimitedMarkerGap;
};

struct Geometry {
    std::vector<Band> bands;
    std::vector<Marker> markers;

    void clear() {
        bands.clear();
        markers.clear();
    }
};

using PositionResolver = std::function<float(double frame)>;

float timeToLevelX(PlayLayer* layer, float seconds);
float invertLevelX(PlayLayer* layer, float targetX);

void assignMarkerSpacing(Geometry& geometry);

Geometry buildGeometry(ReplayData const& replay, double offsetFrames, float anchorX,
                       PositionResolver const& resolver, bool includeReleases);

} // namespace cgv
