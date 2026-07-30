#include "JumpBuffer.hpp"

#include <algorithm>
#include <cmath>

namespace cgv {

void JumpBuffer::setTargets(std::vector<double> times) {
    std::stable_sort(times.begin(), times.end());
    m_times = std::move(times);
    m_consumed.assign(m_times.size(), false);
    m_pending.clear();
}

void JumpBuffer::clearTargets() {
    m_times.clear();
    m_consumed.clear();
    m_pending.clear();
}

void JumpBuffer::resetConsumed() {
    m_consumed.assign(m_times.size(), false);
    m_pending.clear();
}

void JumpBuffer::clearPending() {
    m_pending.clear();
}

size_t JumpBuffer::pendingCount() const {
    size_t count = 0;
    for (auto const& jump : m_pending) {
        if (jump.pending) ++count;
    }
    return count;
}

size_t JumpBuffer::nearestUnconsumed(double songTime, double windowSeconds, double& targetOut) const {
    auto lower = std::lower_bound(m_times.begin(), m_times.end(), songTime);

    for (size_t index = static_cast<size_t>(lower - m_times.begin()); index < m_times.size(); ++index) {
        double ahead = m_times[index] - songTime;
        if (ahead > windowSeconds) break;
        if (index < m_consumed.size() && m_consumed[index]) continue;

        targetOut = m_times[index];
        return index;
    }

    return m_times.size();
}

bool JumpBuffer::tryBuffer(double songTime, double windowSeconds, bool player2) {
    if (m_times.empty()) return false;
    if (pendingCount() >= kMaxBufferedJumps) return false;

    double target = 0.0;
    size_t index = nearestUnconsumed(songTime, windowSeconds, target);
    if (index >= m_times.size()) return false;

    double delay = target - songTime;
    if (delay <= 0.0) return false;
    if (delay > kMaximumHoldSeconds) return false;

    if (index < m_consumed.size()) m_consumed[index] = true;

    BufferedJump jump;
    jump.releaseTime = target;
    jump.player2 = player2;
    jump.pending = true;

    for (auto& slot : m_pending) {
        if (!slot.pending) {
            slot = jump;
            return true;
        }
    }

    m_pending.push_back(jump);
    return true;
}

bool JumpBuffer::takeDueJump(double songTime, bool& player2Out) {
    for (auto& jump : m_pending) {
        if (!jump.pending) continue;
        if (songTime + 1e-9 < jump.releaseTime) continue;

        player2Out = jump.player2;
        jump.pending = false;
        return true;
    }
    return false;
}

} // namespace cgv
