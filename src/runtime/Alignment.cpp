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
    m_presses = 0;
    m_support = 0;
    m_offsetFrames = 0.0;
    m_locked = false;
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
