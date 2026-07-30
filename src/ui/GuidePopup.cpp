#include "GuidePopup.hpp"

#include "core/Compat.hpp"
#include "settings/Settings.hpp"
#include "store/MacroStore.hpp"
#include "ui/MacroPicker.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;
using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kPopupWidth = 380.f;
constexpr float kPopupHeight = 260.f;
constexpr float kListWidth = 320.f;
constexpr float kListHeight = 130.f;
constexpr float kRowHeight = 30.f;
constexpr float kRowGap = 2.f;
constexpr ccColor3B kLoadedColor{80, 255, 140};

std::string abbreviate(std::string const& text, size_t limit) {
    if (text.size() <= limit) return text;
    return text.substr(0, limit - 3) + "...";
}

} // namespace

GuidePopup* GuidePopup::create() {
    auto* popup = new (std::nothrow) GuidePopup();
    if (popup && popup->init()) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool GuidePopup::init() {
    if (!Popup::init(kPopupWidth, kPopupHeight, "GJ_square01.png")) return false;

    this->setTitle("Click Guide");
    buildActionRow();
    buildScrollList();
    buildStatusLabel();
    rebuildList();
    refreshStatus();

    return true;
}

void GuidePopup::buildActionRow() {
    auto* importSprite = ButtonSprite::create("Import Macro", "goldFont.fnt", "GJ_button_01.png", .7f);
    importSprite->setScale(.72f);
    auto* importButton = CCMenuItemSpriteExtra::create(importSprite, this, menu_selector(GuidePopup::onImport));
    m_buttonMenu->addChildAtPosition(importButton, Anchor::Top, ccp(-104, -30));

    auto* folderSprite = ButtonSprite::create("Folder", "goldFont.fnt", "GJ_button_05.png", .7f);
    folderSprite->setScale(.72f);
    auto* folderButton = CCMenuItemSpriteExtra::create(folderSprite, this, menu_selector(GuidePopup::onFolder));
    m_buttonMenu->addChildAtPosition(folderButton, Anchor::Top, ccp(-8, -30));

    auto* settingsSprite = ButtonSprite::create("Settings", "goldFont.fnt", "GJ_button_02.png", .7f);
    settingsSprite->setScale(.72f);
    auto* settingsButton = CCMenuItemSpriteExtra::create(settingsSprite, this, menu_selector(GuidePopup::onSettings));
    m_buttonMenu->addChildAtPosition(settingsButton, Anchor::Top, ccp(92, -30));
}

void GuidePopup::buildScrollList() {
    m_scroll = ScrollLayer::create({kListWidth, kListHeight});
    if (!m_scroll) return;

    auto* background = CCLayerColor::create({0, 0, 0, 90}, kListWidth, kListHeight);
    background->setAnchorPoint(ccp(.5f, .5f));
    m_mainLayer->addChildAtPosition(background, Anchor::Center, ccp(-kListWidth / 2.f, -kListHeight / 2.f + 6.f));

    m_scroll->setAnchorPoint(ccp(.5f, .5f));
    m_mainLayer->addChildAtPosition(m_scroll, Anchor::Center, ccp(-kListWidth / 2.f, -kListHeight / 2.f + 6.f));
}

void GuidePopup::buildStatusLabel() {
    m_status = CCLabelBMFont::create("", "chatFont.fnt");
    if (!m_status) return;
    m_status->setScale(.6f);
    m_mainLayer->addChildAtPosition(m_status, Anchor::Bottom, ccp(0, 24));
}

void GuidePopup::rebuildList() {
    if (!m_scroll || !m_scroll->m_contentLayer) return;

    m_scroll->m_contentLayer->removeAllChildren();
    m_entries = MacroLibrary::list();

    auto const& loadedPath = MacroStore::get().path();
    float totalHeight = std::max(kListHeight, m_entries.size() * (kRowHeight + kRowGap));
    m_scroll->m_contentLayer->setContentSize({kListWidth, totalHeight});

    if (m_entries.empty()) {
        auto* empty = CCLabelBMFont::create("No macros imported yet", "chatFont.fnt");
        empty->setScale(.6f);
        empty->setPosition(ccp(kListWidth / 2.f, totalHeight - kRowHeight));
        m_scroll->m_contentLayer->addChild(empty);
        m_scroll->moveToTop();
        return;
    }

    for (size_t index = 0; index < m_entries.size(); ++index) {
        auto const& entry = m_entries[index];
        bool isLoaded = !loadedPath.empty() && loadedPath == entry.path;

        float rowY = totalHeight - (index + 0.5f) * (kRowHeight + kRowGap);

        auto* name = CCLabelBMFont::create(abbreviate(entry.name, 28).c_str(), "chatFont.fnt");
        name->setScale(.55f);
        name->setAnchorPoint(ccp(0.f, .5f));
        name->setPosition(ccp(10.f, rowY));
        if (isLoaded) name->setColor(kLoadedColor);
        m_scroll->m_contentLayer->addChild(name);

        auto* menu = CCMenu::create();
        menu->setPosition(ccp(0.f, 0.f));

        auto* sprite = ButtonSprite::create(isLoaded ? "Loaded" : "Load", "goldFont.fnt",
                                            isLoaded ? "GJ_button_02.png" : "GJ_button_01.png", .6f);
        sprite->setScale(.6f);

        auto* button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(GuidePopup::onSelectEntry));
        button->setTag(static_cast<int>(index));
        button->setPosition(ccp(kListWidth - 40.f, rowY));
        menu->addChild(button);

        m_scroll->m_contentLayer->addChild(menu);
    }

    m_scroll->moveToTop();
}

void GuidePopup::refreshStatus() {
    if (!m_status) return;

    auto& store = MacroStore::get();
    if (!store.hasMacro()) {
        m_status->setString("No macro loaded");
        return;
    }

    m_status->setString(fmt::format("{} - {} inputs @ {:.0f} FPS",
                                    abbreviate(store.displayName(), 24),
                                    store.inputCount(),
                                    store.framerate())
                            .c_str());
}

void GuidePopup::loadMacroAt(std::filesystem::path const& path) {
    std::string error;
    if (!MacroStore::get().loadFromFile(path, error)) {
        FLAlertLayer::create("Load Failed", error, "OK")->show();
        return;
    }

    Mod::get()->setSavedValue<std::string>("last-macro", path.string());
    rebuildList();
    refreshStatus();
}

void GuidePopup::onSelectEntry(CCObject* sender) {
    auto* button = static_cast<CCMenuItemSpriteExtra*>(sender);
    size_t index = static_cast<size_t>(button->getTag());
    if (index >= m_entries.size()) return;
    loadMacroAt(m_entries[index].path);
}

void GuidePopup::onImport(CCObject*) {
    WeakRef<GuidePopup> self = this;

    pickMacroFile([self](std::optional<std::filesystem::path> picked) mutable {
        if (!picked.has_value()) return;

        std::filesystem::path destination;
        std::string error;
        if (!MacroLibrary::importFile(*picked, destination, error)) {
            FLAlertLayer::create("Import Failed", error, "OK")->show();
            return;
        }

        auto popup = self.lock();
        if (!popup) {
            std::string loadError;
            MacroStore::get().loadFromFile(destination, loadError);
            return;
        }

        popup->loadMacroAt(destination);
    });
}

void GuidePopup::onFolder(CCObject*) {
    MacroLibrary::ensureDirectory();
    auto directory = MacroLibrary::directory();

#ifdef GEODE_IS_ANDROID
    FLAlertLayer::create("Macro Folder", directory.string(), "OK")->show();
#else
    file::openFolder(directory);
#endif
}

void GuidePopup::onSettings(CCObject*) {
    openSettingsPopup(Mod::get());
}

} // namespace cgv
