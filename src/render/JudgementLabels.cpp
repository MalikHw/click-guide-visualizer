#include "JudgementLabels.hpp"

#include "render/GuidePainter.hpp"

#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;
using namespace cocos2d;

namespace cgv {

namespace {

constexpr const char* kLabelFont = "bigFont.fnt";

std::string formatDelta(int deltaFrames) {
    if (deltaFrames >= 0) return fmt::format("+{}f", deltaFrames);
    return fmt::format("{}f", deltaFrames);
}

} // namespace

void JudgementLabels::attachTo(CCNode* parent, int zOrder) {
    if (!parent || !m_pool.empty()) return;

    m_pool.reserve(kLabelPoolSize);
    for (size_t index = 0; index < kLabelPoolSize; ++index) {
        auto* label = CCLabelBMFont::create("", kLabelFont);
        if (!label) continue;
        label->setScale(kLabelScale);
        label->setVisible(false);
        label->setAnchorPoint(ccp(0.5f, 0.f));
        parent->addChild(label, zOrder);
        m_pool.push_back(label);
    }
}

void JudgementLabels::detach() {
    m_pool.clear();
    m_used = 0;
}

void JudgementLabels::beginFrame() {
    m_used = 0;
}

void JudgementLabels::place(float levelX, float levelY, int deltaFrames, float lifeRatio,
                            bool provisional) {
    if (m_used >= m_pool.size()) return;
    if (!std::isfinite(levelX) || !std::isfinite(levelY)) return;

    auto* label = m_pool[m_used++];
    if (!label) return;

    float clampedLife = std::clamp(lifeRatio, 0.f, 1.f);

    label->setString(formatDelta(deltaFrames).c_str());
    label->setColor(judgementColor(deltaFrames));

    float opacity = clampedLife * 255.f;
    if (provisional) opacity *= kProvisionalOpacityScale;
    label->setOpacity(static_cast<GLubyte>(opacity));
    label->setPosition(ccp(levelX, levelY + (1.f - clampedLife) * kLabelRiseUnits));
    label->setVisible(true);
}

void JudgementLabels::endFrame() {
    for (size_t index = m_used; index < m_pool.size(); ++index) {
        if (m_pool[index]) m_pool[index]->setVisible(false);
    }
}

} // namespace cgv
