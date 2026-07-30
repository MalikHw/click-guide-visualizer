#include "GuideNode.hpp"

#include <utility>

using namespace cocos2d;

namespace cgv {

GuideNode* GuideNode::create() {
    auto* node = new (std::nothrow) GuideNode();
    if (node && node->initGuideNode()) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool GuideNode::initGuideNode() {
    if (!CCDrawNode::init()) return false;
    applyAlphaBlending();
    return true;
}

void GuideNode::applyAlphaBlending() {
    ccBlendFunc blend{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
    this->setBlendFunc(blend);
}

void GuideNode::setBeforeDraw(std::function<void()> callback) {
    m_beforeDraw = std::move(callback);
}

void GuideNode::draw() {
    if (m_beforeDraw) m_beforeDraw();
    CCDrawNode::draw();
}

void GuideNode::onExit() {
    m_beforeDraw = nullptr;
    CCDrawNode::onExit();
}

} // namespace cgv
