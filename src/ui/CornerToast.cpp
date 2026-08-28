#include "CornerToast.hpp"

#include "settings/Settings.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace cocos2d;

namespace cgv {

namespace {

constexpr const char* kToastId = "corner-toast"_spr;
constexpr const char* kToastFont = "bigFont.fnt";

CCNode* toastHost() {
    auto* director = CCDirector::sharedDirector();
    if (!director) return nullptr;
    return director->getRunningScene();
}

void removeExisting(CCNode* host) {
    if (auto* old = host->getChildByID(kToastId)) {
        old->removeFromParent();
    }
}

CCActionInterval* fadeSequence(float visibleSeconds) {
    return CCSequence::create(
        CCEaseSineOut::create(CCFadeIn::create(kToastFadeSeconds)),
        CCDelayTime::create(visibleSeconds),
        CCEaseSineIn::create(CCFadeOut::create(kToastFadeSeconds)),
        nullptr);
}

CCActionInterval* driftSequence(float visibleSeconds) {
    return CCSequence::create(
        CCEaseOut::create(CCMoveBy::create(kToastFadeSeconds, ccp(0.f, -kToastDrift)), 2.f),
        CCDelayTime::create(visibleSeconds + kToastFadeSeconds),
        nullptr);
}

} // namespace

void showToast(std::string const& text) {
    showToast(text, ccColor3B{255, 255, 255});
}

void showToast(std::string const& text, ccColor3B colour) {
    if (text.empty()) return;

    auto* host = toastHost();
    if (!host) return;

    removeExisting(host);

    auto* label = CCLabelBMFont::create(text.c_str(), kToastFont);
    if (!label) return;

    CCSize screen = CCDirector::sharedDirector()->getWinSize();

    label->setID(kToastId);
    label->setScale(kToastScale);
    label->limitLabelWidth(screen.width * 0.5f, kToastScale, kToastScale * 0.5f);
    label->setColor(colour);
    label->setAnchorPoint(ccp(1.f, 1.f));
    label->setPosition(ccp(screen.width - kToastMargin, screen.height - kToastMargin + kToastDrift));
    label->setOpacity(0);

    host->addChild(label, kToastZOrder);

    float visible = settings().hotkeyPopupSeconds;
    label->runAction(driftSequence(visible));
    label->runAction(CCSequence::create(
        fadeSequence(visible),
        CCCallFunc::create(label, callfunc_selector(CCNode::removeFromParent)),
        nullptr));
}

} // namespace cgv
