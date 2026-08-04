#pragma once

#include "model/ReplayData.hpp"

#include <cstddef>
#include <vector>

namespace cgv {

constexpr double kMinimumHoldSeconds = 0.05;
constexpr double kUnlimitedGapSeconds = 1.0e9;

struct RhythmNote {
    double hitTime = 0.0;
    double releaseTime = 0.0;
    double gapToNext = kUnlimitedGapSeconds;
    bool player2 = false;
    bool isHold = false;
};

void assignNoteSpacing(std::vector<RhythmNote>& timeline);

std::vector<RhythmNote> buildRhythmTimeline(ReplayData const& replay, double offsetSeconds);
std::vector<RhythmNote> timelineFromTimes(std::vector<double> const& times);
std::vector<RhythmNote> timelineFromHolds(std::vector<double> const& times,
                                          std::vector<double> const& holds);

size_t firstNoteAtOrAfter(std::vector<RhythmNote> const& timeline, double time);

} // namespace cgv
