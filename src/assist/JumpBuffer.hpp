#pragma once

#include <cstddef>
#include <vector>

namespace cgv {

struct BufferedJump {
    double releaseTime = 0.0;
    bool player2 = false;
    bool pending = false;
};

constexpr size_t kMaxBufferedJumps = 4;
constexpr double kMaximumHoldSeconds = 0.35;

class JumpBuffer {
public:
    void setTargets(std::vector<double> times);
    void clearTargets();
    void resetConsumed();
    void clearPending();

    bool tryBuffer(double songTime, double windowSeconds, bool player2);
    bool takeDueJump(double songTime, bool& player2Out);

    bool hasTargets() const { return !m_times.empty(); }
    size_t pendingCount() const;

private:
    size_t nearestUnconsumed(double songTime, double windowSeconds, double& targetOut) const;

    std::vector<double> m_times;
    std::vector<bool> m_consumed;
    std::vector<BufferedJump> m_pending;
};

} // namespace cgv
