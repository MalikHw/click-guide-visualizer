#pragma once

#include "assist/JumpBuffer.hpp"

namespace cgv {

class AssistState {
public:
    static AssistState& get();

    JumpBuffer& buffer() { return m_buffer; }
    JumpBuffer const& buffer() const { return m_buffer; }

    void setActive(bool active) { m_active = active; }
    bool active() const { return m_active; }

    void setSongTime(double time) { m_songTime = time; }
    double songTime() const { return m_songTime; }

    void noteBuffered() { ++m_bufferedCount; }
    unsigned long long bufferedCount() const { return m_bufferedCount; }
    void resetCounters() { m_bufferedCount = 0; }

    void setReleasing(bool releasing) { m_releasing = releasing; }
    bool releasing() const { return m_releasing; }

private:
    AssistState() = default;

    JumpBuffer m_buffer;
    bool m_active = false;
    bool m_releasing = false;
    double m_songTime = 0.0;
    unsigned long long m_bufferedCount = 0;
};

bool cheatsAreActive();

} // namespace cgv
