#pragma once

class PlayLayer;

namespace cgv {

constexpr float kStartPosDetectUnits = 12.f;
constexpr float kMaxStartOffsetSeconds = 1200.f;

class StartOffset {
public:
    void reset();
    void measure(PlayLayer* layer);

    bool settled() const { return m_settled; }
    bool fromStartPos() const { return m_seconds > 0.f; }
    float seconds() const { return m_seconds; }

private:
    float m_seconds = 0.f;
    bool m_settled = false;
};

float measureStartOffsetSeconds(PlayLayer* layer);

} // namespace cgv
