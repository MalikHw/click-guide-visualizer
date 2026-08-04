#pragma once

#include <cocos2d.h>
#include <cstddef>
#include <vector>

namespace cgv {

constexpr size_t kLabelPoolSize = 24;
constexpr float kLabelScale = 0.55f;
constexpr float kLabelRiseUnits = 26.f;
constexpr float kProvisionalOpacityScale = 0.55f;

class JudgementLabels {
public:
    void attachTo(cocos2d::CCNode* parent, int zOrder);
    void detach();

    void beginFrame();
    void place(float levelX, float levelY, int deltaFrames, float lifeRatio, bool provisional);
    void endFrame();

private:
    std::vector<cocos2d::CCLabelBMFont*> m_pool;
    size_t m_used = 0;
};

} // namespace cgv
