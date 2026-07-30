#pragma once

#include <cstddef>
#include <vector>

namespace cgv {

constexpr size_t kMinAlignmentSamples = 5;
constexpr size_t kMaxAlignmentSamples = 15;
constexpr double kAlignmentSpreadFrames = 8.0;
constexpr double kSearchWindowSeconds = 3.0;
constexpr double kLockedWindowSeconds = 0.5;

class Alignment {
public:
    void reset();
    bool addSample(double deltaFrames);

    bool locked() const { return m_locked; }
    double offsetFrames() const { return m_offsetFrames; }
    double windowSeconds() const { return m_locked ? kLockedWindowSeconds : kSearchWindowSeconds; }
    size_t sampleCount() const { return m_samples.size(); }

private:
    bool tryLock();

    std::vector<double> m_samples;
    double m_offsetFrames = 0.0;
    bool m_locked = false;
};

double medianOf(std::vector<double> values);

} // namespace cgv
