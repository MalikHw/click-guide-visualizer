#include "runtime/Runtime.hpp"
#include "ui/GuidePopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

namespace {

constexpr const char* kButtonId = "click-guide-button"_spr;
constexpr const char* kFallbackMenuId = "click-guide-menu"_spr;
constexpr float kFallbackMenuOffset = 60.f;

CCMenu* resolveHostMenu(PauseLayer* layer) {
    if (auto* column = layer->getChildByID("right-button-menu")) {
        return typeinfo_cast<CCMenu*>(column);
    }

    auto* fallback = CCMenu::create();
    fallback->setID(kFallbackMenuId);
    fallback->setPosition(CCDirector::sharedDirector()->getWinSize() - ccp(kFallbackMenuOffset, kFallbackMenuOffset));
    layer->addChild(fallback);
    return fallback;
}

} // namespace

struct GuidePauseLayer : geode::Modify<GuidePauseLayer, PauseLayer> {
    void customSetup() {
        PauseLayer::customSetup();

        if (!this->getChildByIDRecursive(kButtonId)) {
            auto* host = resolveHostMenu(this);
            if (host) {
                auto* sprite = CircleButtonSprite::createWithSpriteFrameName(
                    "GJ_hammerIcon_001.png", 1.f, CircleBaseColor::Green, CircleBaseSize::Medium);
                auto* button = CCMenuItemSpriteExtra::create(sprite, this,
                                                             menu_selector(GuidePauseLayer::onClickGuide));
                button->setID(kButtonId);
                host->addChild(button);
                host->updateLayout();
            }
        }

        Loader::get()->queueInMainThread([]() { cgv::Runtime::get().requestRedraw(); });
    }

    void onClickGuide(CCObject*) {
        if (auto* popup = cgv::GuidePopup::create()) popup->show();
    }
};
