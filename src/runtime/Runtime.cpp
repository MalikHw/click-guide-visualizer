#include "Runtime.hpp"

#include <utility>

namespace cgv {

Runtime& Runtime::get() {
    static Runtime instance;
    return instance;
}

void Runtime::queuePress(int frame, bool player2) {
    if (m_presses.size() >= kMaxQueuedPresses) {
        m_presses.erase(m_presses.begin());
    }
    m_presses.push_back(PressEvent{frame, player2});
}

std::vector<PressEvent> Runtime::drainPresses() {
    std::vector<PressEvent> drained;
    drained.swap(m_presses);
    return drained;
}

void Runtime::clearPresses() {
    m_presses.clear();
}

void Runtime::addJudgement(Judgement judgement) {
    judgement.remainingSeconds = kJudgementLifetimeSeconds;
    m_judgements.push_back(judgement);
    while (m_judgements.size() > kMaxActiveJudgements) {
        m_judgements.pop_front();
    }
}

void Runtime::ageJudgements(float deltaSeconds) {
    for (auto& judgement : m_judgements) {
        judgement.remainingSeconds -= deltaSeconds;
    }
    while (!m_judgements.empty() && m_judgements.front().remainingSeconds <= 0.f) {
        m_judgements.pop_front();
    }
}

void Runtime::clearJudgements() {
    m_judgements.clear();
}

void Runtime::setRedrawHandler(std::function<void()> handler) {
    m_redraw = std::move(handler);
}

void Runtime::clearRedrawHandler() {
    m_redraw = nullptr;
}

void Runtime::requestRedraw() {
    if (m_redraw) m_redraw();
}

} // namespace cgv
