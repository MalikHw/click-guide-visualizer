#include "RhythmModel.hpp"

#include <algorithm>
#include <cmath>

namespace cgv {

namespace {

struct PendingHold {
    size_t noteIndex = 0;
    bool active = false;
};

} // namespace

std::vector<RhythmNote> buildRhythmTimeline(ReplayData const& replay, double offsetSeconds) {
    std::vector<RhythmNote> timeline;
    if (replay.inputs.empty()) return timeline;

    double tickRate = replay.framerate > 0.0 ? replay.framerate : 240.0;
    timeline.reserve(replay.inputs.size() / 2 + 1);

    PendingHold pendingPlayer1;
    PendingHold pendingPlayer2;

    for (auto const& input : replay.inputs) {
        double time = static_cast<double>(input.frame) / tickRate + offsetSeconds;
        if (!std::isfinite(time)) continue;

        PendingHold& pending = input.player2 ? pendingPlayer2 : pendingPlayer1;

        if (input.down) {
            RhythmNote note;
            note.hitTime = time;
            note.releaseTime = time;
            note.player2 = input.player2;
            note.isHold = false;
            timeline.push_back(note);

            pending.noteIndex = timeline.size() - 1;
            pending.active = true;
            continue;
        }

        if (!pending.active) continue;

        RhythmNote& note = timeline[pending.noteIndex];
        note.releaseTime = std::max(time, note.hitTime);
        note.isHold = (note.releaseTime - note.hitTime) >= kMinimumHoldSeconds;
        pending.active = false;
    }

    std::stable_sort(timeline.begin(), timeline.end(),
                     [](RhythmNote const& a, RhythmNote const& b) { return a.hitTime < b.hitTime; });

    return timeline;
}

std::vector<RhythmNote> timelineFromTimes(std::vector<double> const& times) {
    return timelineFromHolds(times, {});
}

std::vector<RhythmNote> timelineFromHolds(std::vector<double> const& times,
                                          std::vector<double> const& holds) {
    std::vector<RhythmNote> timeline;
    timeline.reserve(times.size());

    for (size_t index = 0; index < times.size(); ++index) {
        double time = times[index];
        if (!std::isfinite(time)) continue;

        double hold = index < holds.size() ? holds[index] : 0.0;
        if (!std::isfinite(hold) || hold < 0.0) hold = 0.0;

        RhythmNote note;
        note.hitTime = time;
        note.releaseTime = time + hold;
        note.player2 = false;
        note.isHold = hold >= kMinimumHoldSeconds;
        timeline.push_back(note);
    }

    std::stable_sort(timeline.begin(), timeline.end(),
                     [](RhythmNote const& a, RhythmNote const& b) { return a.hitTime < b.hitTime; });

    return timeline;
}

size_t firstNoteAtOrAfter(std::vector<RhythmNote> const& timeline, double time) {
    auto found = std::lower_bound(timeline.begin(), timeline.end(), time,
                                  [](RhythmNote const& note, double value) { return note.hitTime < value; });
    return static_cast<size_t>(found - timeline.begin());
}

} // namespace cgv
