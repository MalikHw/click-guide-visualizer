#pragma once

#include <cocos2d.h>
#include <functional>

namespace cgv {

class GuideNode : public cocos2d::CCDrawNode {
public:
    static GuideNode* create();

    void setBeforeDraw(std::function<void()> callback);

    void draw() override;
    void onExit() override;

private:
    bool initGuideNode();
    void applyAlphaBlending();

    std::function<void()> m_beforeDraw;
};

} // namespace cgv
