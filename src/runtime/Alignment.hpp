#pragma once

#include <cstddef>
#include <vector>

namespace cgv {

constexpr size_t kMinAlignmentVotes = 6;
constexpr size_t kMaxCandidateOffsets = 4096;
constexpr size_t kMinPressesBeforeLock = 6;
constexpr double kAlignmentSpreadFrames = 8.0;
constexpr double kCandidateBinFrames = 4.0;
constexpr double kMinSupportRatio = 0.5;
constexpr double kSearchWindowSeconds = 3.0;
constexpr double kLockedWindowSeconds = 0.5;

class Alignment {
public:
    void reset();
    void addCandidates(std::vector<double> const& deltas);

    bool locked() const { return m_locked; }
    double offsetFrames() const { return m_offsetFrames; }
    double windowSeconds() const { return m_locked ? kLockedWindowSeconds : kSearchWindowSeconds; }
    size_t pressCount() const { return m_presses; }
    size_t supportCount() const { return m_support; }

private:
    bool tryLock();

    struct Candidate {
        double delta = 0.0;
        size_t press = 0;
    };

    std::vector<Candidate> m_candidates;
    size_t m_presses = 0;
    size_t m_support = 0;
    double m_offsetFrames = 0.0;
    bool m_locked = false;
};

double medianOf(std::vector<double> values);

} // namespace cgv
