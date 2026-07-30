#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <vector>

namespace cgv {

struct PressEvent {
    int frame = 0;
    bool player2 = false;
};

struct Judgement {
    float levelX = 0.f;
    float levelY = 0.f;
    int deltaFrames = 0;
    float remainingSeconds = 0.f;
};

constexpr size_t kMaxQueuedPresses = 32;
constexpr size_t kMaxActiveJudgements = 12;
constexpr float kJudgementLifetimeSeconds = 1.1f;
constexpr int kPerfectFrameWindow = 1;
constexpr int kGoodFrameWindow = 4;

class Runtime {
public:
    static Runtime& get();

    void queuePress(int frame, bool player2);
    std::vector<PressEvent> drainPresses();
    void clearPresses();

    void addJudgement(Judgement judgement);
    void ageJudgements(float deltaSeconds);
    void clearJudgements();
    std::deque<Judgement> const& judgements() const { return m_judgements; }

    void setRedrawHandler(std::function<void()> handler);
    void clearRedrawHandler();
    void requestRedraw();

private:
    Runtime() = default;

    std::vector<PressEvent> m_presses;
    std::deque<Judgement> m_judgements;
    std::function<void()> m_redraw;
};

} // namespace cgv
