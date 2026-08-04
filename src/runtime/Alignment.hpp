#pragma once

#include <cstddef>
#include <vector>

namespace cgv {

constexpr size_t kMinAlignmentVotes = 6;
constexpr size_t kMaxCandidateOffsets = 4096;
constexpr size_t kMinPressesBeforeLock = 6;
constexpr size_t kMinPressesBeforeProvisional = 2;
constexpr double kProvisionalAgreementFrames = 12.0;
constexpr double kAlignmentSpreadFrames = 8.0;
constexpr double kCandidateBinFrames = 4.0;
constexpr double kMinSupportRatio = 0.5;
constexpr double kSearchWindowSeconds = 3.0;
constexpr double kLockedWindowSeconds = 0.5;

constexpr size_t kDriftSampleCount = 8;
constexpr double kDriftTriggerFrames = 6.0;
constexpr double kMinRateCorrectionSeconds = 4.0;
constexpr double kMaxRateCorrection = 0.25;
constexpr double kAssumedTickRate = 240.0;

class Alignment {
public:
    void reset();
    void addCandidates(std::vector<double> const& deltas);

    void observeResidual(double residualFrames, double songSeconds);
    bool hasRateCorrection() const { return m_rateCorrected; }
    double rateScale() const { return m_rateScale; }

    bool locked() const { return m_locked; }
    double offsetFrames() const { return m_offsetFrames; }
    double windowSeconds() const { return m_locked ? kLockedWindowSeconds : kSearchWindowSeconds; }
    size_t pressCount() const { return m_presses; }
    size_t supportCount() const { return m_support; }
    bool driftDetected() const { return m_driftDetected; }
    bool provisionalTrustworthy() const;

private:
    struct Candidate {
        double delta = 0.0;
        size_t press = 0;
    };

    struct Residual {
        double frames = 0.0;
        double seconds = 0.0;
    };

    bool tryLock();
    void evaluateDrift();
    void relock();

    std::vector<Candidate> m_candidates;
    std::vector<Residual> m_residuals;
    size_t m_presses = 0;
    size_t m_support = 0;
    double m_offsetFrames = 0.0;
    double m_rateScale = 1.0;
    bool m_rateCorrected = false;
    bool m_driftDetected = false;
    bool m_locked = false;
};

double medianOf(std::vector<double> values);
double slopeOf(std::vector<double> const& xs, std::vector<double> const& ys);

} // namespace cgv
