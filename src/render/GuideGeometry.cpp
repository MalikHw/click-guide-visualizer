#include "GuideGeometry.hpp"

#include <Geode/Bindings.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace cgv {

namespace {

constexpr float kMaxBisectionTime = 100000.f;

struct PendingPress {
    size_t inputIndex = 0;
    double frame = 0.0;
    bool active = false;
};

double alignedFrame(uint64_t rawFrame, double offsetFrames) {
    return static_cast<double>(rawFrame) + offsetFrames;
}

void emitBand(Geometry& geometry, PendingPress const& press, double releaseFrame,
              PositionResolver const& resolver, bool player2) {
    float pressX = resolver(press.frame);
    float releaseX = resolver(releaseFrame);

    if (!std::isfinite(pressX) || !std::isfinite(releaseX)) return;

    Band band;
    band.pressX = pressX;
    band.x0 = pressX;
    band.x1 = std::max(releaseX, pressX + kMinimumBandWidth);
    band.player2 = player2;
    band.inputIndex = press.inputIndex;
    geometry.bands.push_back(band);
}

void normaliseBands(Geometry& geometry) {
    for (auto& band : geometry.bands) {
        if (band.x1 < band.x0) std::swap(band.x0, band.x1);
        if (band.x1 - band.x0 < kMinimumBandWidth) band.x1 = band.x0 + kMinimumBandWidth;
    }
}

void sortByX(Geometry& geometry) {
    normaliseBands(geometry);

    std::stable_sort(geometry.bands.begin(), geometry.bands.end(),
                     [](Band const& a, Band const& b) { return a.x0 < b.x0; });
    std::stable_sort(geometry.markers.begin(), geometry.markers.end(),
                     [](Marker const& a, Marker const& b) { return a.x < b.x; });
}

} // namespace

float timeToLevelX(PlayLayer* layer, float seconds) {
    if (!layer) return 0.f;
    if (seconds < 0.f) seconds = 0.f;
    return layer->posForTime(seconds).x;
}

float invertLevelX(PlayLayer* layer, float targetX) {
    if (!layer) return 0.f;

    float low = 0.f;
    float high = kBisectionInitialSpan;

    for (int expansion = 0; expansion < kBisectionExpansions; ++expansion) {
        if (timeToLevelX(layer, high) >= targetX) break;
        low = high;
        high *= 2.f;
        if (high > kMaxBisectionTime) break;
    }

    for (int step = 0; step < kBisectionHalvings; ++step) {
        float mid = (low + high) * 0.5f;
        if (timeToLevelX(layer, mid) < targetX) {
            low = mid;
        } else {
            high = mid;
        }
    }

    return (low + high) * 0.5f;
}

void assignMarkerSpacing(Geometry& geometry) {
    auto& markers = geometry.markers;

    for (size_t index = 0; index < markers.size(); ++index) {
        float gap = kUnlimitedMarkerGap;

        for (size_t ahead = index + 1; ahead < markers.size(); ++ahead) {
            if (markers[ahead].player2 != markers[index].player2) continue;
            gap = std::min(gap, std::fabs(markers[ahead].x - markers[index].x));
            break;
        }

        for (size_t behind = index; behind-- > 0;) {
            if (markers[behind].player2 != markers[index].player2) continue;
            gap = std::min(gap, std::fabs(markers[index].x - markers[behind].x));
            break;
        }

        if (!std::isfinite(gap) || gap <= 0.f) gap = kUnlimitedMarkerGap;
        markers[index].gapToNeighbour = gap;
    }
}

Geometry buildGeometry(ReplayData const& replay, double offsetFrames, float anchorX,
                       PositionResolver const& resolver, bool includeReleases) {
    Geometry geometry;
    if (replay.inputs.empty()) return geometry;

    const bool useRecorded = replay.allPositions;

    auto resolveX = [&](ReplayInput const& input) -> float {
        if (useRecorded) return input.xPos + anchorX;
        return resolver(alignedFrame(input.frame, offsetFrames));
    };

    geometry.bands.reserve(replay.inputs.size() / 2 + 1);
    geometry.markers.reserve(replay.inputs.size());

    PendingPress pendingPlayer1;
    PendingPress pendingPlayer2;

    for (size_t index = 0; index < replay.inputs.size(); ++index) {
        auto const& input = replay.inputs[index];
        float x = resolveX(input);
        if (!std::isfinite(x)) continue;

        if (input.down || includeReleases) {
            Marker marker;
            marker.x = x;
            marker.player2 = input.player2;
            marker.isRelease = !input.down;
            marker.inputIndex = index;
            marker.hasRecordedY = useRecorded && input.hasPosition;
            if (marker.hasRecordedY) marker.y = input.yPos;
            geometry.markers.push_back(marker);
        }

        PendingPress& pending = input.player2 ? pendingPlayer2 : pendingPlayer1;

        if (input.down) {
            pending.active = true;
            pending.inputIndex = index;
            pending.frame = useRecorded
                ? static_cast<double>(input.frame) + offsetFrames
                : alignedFrame(input.frame, offsetFrames);
            if (useRecorded) {
                Band band;
                band.pressX = x;
                band.x0 = x;
                band.x1 = x + kMinimumBandWidth;
                band.player2 = input.player2;
                band.inputIndex = index;
                geometry.bands.push_back(band);
            }
            continue;
        }

        if (!pending.active) continue;

        if (useRecorded) {
            if (!geometry.bands.empty()) {
                for (auto it = geometry.bands.rbegin(); it != geometry.bands.rend(); ++it) {
                    if (it->inputIndex == pending.inputIndex) {
                        it->x1 = std::max(x, it->x0 + kMinimumBandWidth);
                        break;
                    }
                }
            }
        } else {
            emitBand(geometry, pending, alignedFrame(input.frame, offsetFrames), resolver, input.player2);
        }

        pending.active = false;
    }

    sortByX(geometry);
    assignMarkerSpacing(geometry);
    return geometry;
}

} // namespace cgv
