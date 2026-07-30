#include "Alignment.hpp"

#include <algorithm>

namespace cgv {

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
    m_samples.clear();
    m_offsetFrames = 0.0;
    m_locked = false;
}

bool Alignment::addSample(double deltaFrames) {
    if (m_locked) return false;

    m_samples.push_back(deltaFrames);
    if (m_samples.size() > kMaxAlignmentSamples) {
        m_samples.erase(m_samples.begin());
    }

    return tryLock();
}

bool Alignment::tryLock() {
    if (m_samples.size() < kMinAlignmentSamples) return false;

    auto bounds = std::minmax_element(m_samples.begin(), m_samples.end());
    double spread = *bounds.second - *bounds.first;
    if (spread > kAlignmentSpreadFrames) return false;

    m_offsetFrames = medianOf(m_samples);
    m_locked = true;
    return true;
}

} // namespace cgv
