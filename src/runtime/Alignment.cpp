#include "Alignment.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <set>

namespace cgv {

namespace {

long long binIndexOf(double delta) {
    return static_cast<long long>(std::llround(delta / kCandidateBinFrames));
}

template <typename Candidate>
std::vector<Candidate> gatherHeaviestBin(std::vector<Candidate> const& candidates) {
    std::map<long long, std::vector<Candidate>> bins;
    for (auto const& candidate : candidates) {
        bins[binIndexOf(candidate.delta)].push_back(candidate);
    }

    if (bins.empty()) return {};

    auto heaviest = bins.begin();
    for (auto entry = bins.begin(); entry != bins.end(); ++entry) {
        if (entry->second.size() > heaviest->second.size()) {
            heaviest = entry;
            continue;
        }
        if (entry->second.size() < heaviest->second.size()) continue;
        if (std::llabs(entry->first) < std::llabs(heaviest->first)) heaviest = entry;
    }

    std::vector<Candidate> merged = heaviest->second;
    for (long long neighbour : {heaviest->first - 1, heaviest->first + 1}) {
        auto found = bins.find(neighbour);
        if (found == bins.end()) continue;
        merged.insert(merged.end(), found->second.begin(), found->second.end());
    }
    return merged;
}

template <typename Candidate>
std::vector<Candidate> keepNearMedian(std::vector<Candidate> const& values, double median) {
    std::vector<Candidate> kept;
    kept.reserve(values.size());
    for (auto const& value : values) {
        if (std::fabs(value.delta - median) <= kAlignmentSpreadFrames) kept.push_back(value);
    }
    return kept;
}

template <typename Candidate>
std::vector<double> deltasOf(std::vector<Candidate> const& candidates) {
    std::vector<double> deltas;
    deltas.reserve(candidates.size());
    for (auto const& candidate : candidates) deltas.push_back(candidate.delta);
    return deltas;
}

template <typename Candidate>
size_t distinctPressesIn(std::vector<Candidate> const& candidates) {
    std::set<size_t> presses;
    for (auto const& candidate : candidates) presses.insert(candidate.press);
    return presses.size();
}

} // namespace

double slopeOf(std::vector<double> const& xs, std::vector<double> const& ys) {
    if (xs.size() != ys.size() || xs.size() < 2) return 0.0;

    double meanX = 0.0;
    double meanY = 0.0;
    for (size_t index = 0; index < xs.size(); ++index) {
        meanX += xs[index];
        meanY += ys[index];
    }
    meanX /= static_cast<double>(xs.size());
    meanY /= static_cast<double>(ys.size());

    double covariance = 0.0;
    double variance = 0.0;
    for (size_t index = 0; index < xs.size(); ++index) {
        double dx = xs[index] - meanX;
        covariance += dx * (ys[index] - meanY);
        variance += dx * dx;
    }

    if (variance <= 0.0) return 0.0;
    return covariance / variance;
}

double medianOf(std::vector<double> values) {
    if (values.empty()) return 0.0;
    size_t middle = values.size() / 2;
    std::nth_element(values.begin(), values.begin() + middle, values.end());
    double upper = values[middle];
    if (values.size() % 2 != 0) return upper;

    std::nth_element(values.begin(), values.begin() + middle - 1, values.end());
    return (upper + values[middle - 1]) * 0.5;
}

void Alignment::reset() {
    m_candidates.clear();
    m_residuals.clear();
    m_presses = 0;
    m_support = 0;
    m_offsetFrames = 0.0;
    m_rateScale = 1.0;
    m_rateCorrected = false;
    m_driftDetected = false;
    m_locked = false;
}

void Alignment::relock() {
    m_candidates.clear();
    m_residuals.clear();
    m_presses = 0;
    m_support = 0;
    m_locked = false;
}

void Alignment::observeResidual(double residualFrames, double songSeconds) {
    if (!m_locked) return;
    if (!std::isfinite(residualFrames) || !std::isfinite(songSeconds)) return;

    m_residuals.push_back(Residual{residualFrames, songSeconds});
    if (m_residuals.size() > kDriftSampleCount) {
        m_residuals.erase(m_residuals.begin());
    }

    evaluateDrift();
}

void Alignment::evaluateDrift() {
    if (m_residuals.size() < kDriftSampleCount) return;

    double elapsed = m_residuals.back().seconds - m_residuals.front().seconds;
    if (elapsed < kMinRateCorrectionSeconds) return;

    double spread = std::fabs(m_residuals.back().frames - m_residuals.front().frames);
    if (spread < kDriftTriggerFrames) return;

    std::vector<double> seconds;
    std::vector<double> frames;
    seconds.reserve(m_residuals.size());
    frames.reserve(m_residuals.size());
    for (auto const& residual : m_residuals) {
        seconds.push_back(residual.seconds);
        frames.push_back(residual.frames);
    }

    double framesPerSecond = slopeOf(seconds, frames);
    if (!std::isfinite(framesPerSecond) || framesPerSecond == 0.0) return;


    double correction = 1.0 + framesPerSecond / kAssumedTickRate;
    if (!std::isfinite(correction)) return;

    double proposed = m_rateScale * correction;
    if (proposed < 1.0 - kMaxRateCorrection || proposed > 1.0 + kMaxRateCorrection) return;

    m_rateScale = proposed;
    m_driftDetected = true;
    m_rateCorrected = true;
    relock();
}

void Alignment::addCandidates(std::vector<double> const& deltas) {
    if (m_locked) return;
    if (deltas.empty()) return;

    ++m_presses;

    for (double delta : deltas) {
        if (m_candidates.size() >= kMaxCandidateOffsets) break;
        m_candidates.push_back(Candidate{delta, m_presses});
    }

    tryLock();
}

bool Alignment::provisionalTrustworthy() const {
    if (m_locked) return true;
    if (m_presses < kMinPressesBeforeProvisional) return false;
    if (m_candidates.empty()) return false;

    auto peak = gatherHeaviestBin(m_candidates);
    if (peak.empty()) return false;

    double centre = medianOf(deltasOf(peak));
    if (std::fabs(centre) > kProvisionalAgreementFrames) return false;

    auto kept = keepNearMedian(peak, centre);
    return distinctPressesIn(kept) >= m_presses;
}

bool Alignment::tryLock() {
    if (m_presses < kMinPressesBeforeLock) return false;
    if (m_candidates.size() < kMinAlignmentVotes) return false;

    auto peak = gatherHeaviestBin(m_candidates);
    if (peak.size() < kMinAlignmentVotes) return false;

    auto kept = keepNearMedian(peak, medianOf(deltasOf(peak)));
    m_support = distinctPressesIn(kept);

    if (m_support < kMinAlignmentVotes) return false;

    double ratio = static_cast<double>(m_support) / static_cast<double>(m_presses);
    if (ratio < kMinSupportRatio) return false;

    m_offsetFrames = medianOf(deltasOf(kept));
    m_locked = true;
    return true;
}

} // namespace cgv
