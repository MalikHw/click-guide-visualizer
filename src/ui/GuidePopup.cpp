#include "GuidePopup.hpp"

#include "core/Compat.hpp"
#include "settings/Settings.hpp"
#include "runtime/Alignment.hpp"
#include "runtime/Runtime.hpp"
#include "store/MacroSetting.hpp"
#include "online/HyperbolusClient.hpp"
#include "store/MacroStore.hpp"
#include "ui/MacroPicker.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <algorithm>
#include <cstddef>

using namespace geode::prelude;
using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kPopupWidth = 420.f;
constexpr float kPopupHeight = 290.f;
constexpr float kListWidth = 368.f;
constexpr float kListHeight = 158.f;
constexpr float kRowHeight = 32.f;
constexpr float kRowGap = 3.f;
constexpr float kTitleOffset = 22.f;
constexpr float kTitleScale = .8f;
constexpr float kButtonRowY = -58.f;
constexpr float kListCentreY = -20.f;
constexpr float kStatusY = 18.f;
constexpr float kRowLabelInset = 12.f;
constexpr float kRowButtonInset = 48.f;
constexpr float kRowNameGap = 34.f;
constexpr size_t kNameLimit = 30;
constexpr const char* kBoldFont = "bigFont.fnt";
constexpr float kRowNameScale = .42f;
constexpr float kRowNameMinScale = .26f;
constexpr float kStatusScale = .34f;
constexpr float kStatusMinScale = .22f;
constexpr float kEmptyScale = .45f;
constexpr ccColor3B kLoadedColor{110, 245, 165};
constexpr ccColor4B kListBackground{0, 0, 0, 88};
constexpr ccColor4B kRowTint{255, 255, 255, 9};

float rowNameWidth() {
    return kListWidth - kRowButtonInset - kRowLabelInset - kRowNameGap;
}

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

    this->setTitle("Click Guide", "goldFont.fnt", kTitleScale, kTitleOffset);
    buildActionRow();
    buildScrollList();
    buildStatusLabel();
    rebuildList();
    refreshStatus();

    return true;
}

void GuidePopup::buildActionRow() {
    struct ActionButton {
        const char* label;
        const char* texture;
        cocos2d::SEL_MenuHandler handler;
        bool isUnload;
    };

    const ActionButton actions[] = {
        {"Import", "GJ_button_01.png", menu_selector(GuidePopup::onImport), false},
        {"Online", "GJ_button_03.png", menu_selector(GuidePopup::onOnline), false},
        {"Folder", "GJ_button_05.png", menu_selector(GuidePopup::onFolder), false},
        {"Unload", "GJ_button_06.png", menu_selector(GuidePopup::onUnload), true},
        {"Settings", "GJ_button_02.png", menu_selector(GuidePopup::onSettings), false},
    };

    constexpr size_t kActionCount = sizeof(actions) / sizeof(actions[0]);
    constexpr float kActionSpacing = 78.f;
    float firstX = -kActionSpacing * (kActionCount - 1) * 0.5f;

    for (size_t index = 0; index < kActionCount; ++index) {
        auto* sprite = ButtonSprite::create(actions[index].label, "goldFont.fnt",
                                            actions[index].texture, .8f);
        if (!sprite) continue;
        sprite->setScale(.62f);

        auto* button = CCMenuItemSpriteExtra::create(sprite, this, actions[index].handler);
        if (!button) continue;

        m_buttonMenu->addChildAtPosition(button, Anchor::Top,
                                         ccp(firstX + kActionSpacing * index, kButtonRowY));

        if (actions[index].isUnload) m_unloadButton = button;
    }
}

void GuidePopup::refreshUnloadButton() {
    if (!m_unloadButton) return;
    bool loaded = MacroStore::get().hasMacro();
    m_unloadButton->setEnabled(loaded);
    m_unloadButton->setOpacity(loaded ? 255 : 100);
    if (auto* sprite = m_unloadButton->getChildByType<ButtonSprite>(0)) {
        sprite->setCascadeOpacityEnabled(true);
        sprite->setOpacity(loaded ? 255 : 100);
    }
}

void GuidePopup::buildScrollList() {
    m_scroll = ScrollLayer::create({kListWidth, kListHeight});
    if (!m_scroll) return;

    auto* background = CCScale9Sprite::create("square02_001.png");
    if (background) {
        background->setContentSize({kListWidth, kListHeight});
        background->setOpacity(kListBackground.a);
        background->setColor({kListBackground.r, kListBackground.g, kListBackground.b});
        m_mainLayer->addChildAtPosition(background, Anchor::Center, ccp(0.f, kListCentreY));
    }

    m_scroll->ignoreAnchorPointForPosition(false);
    m_scroll->setAnchorPoint(ccp(.5f, .5f));
    m_mainLayer->addChildAtPosition(m_scroll, Anchor::Center, ccp(0.f, kListCentreY));
}

void GuidePopup::buildStatusLabel() {
    m_status = CCLabelBMFont::create("", kBoldFont);
    if (!m_status) return;
    m_status->setScale(kStatusScale);
    m_status->setAnchorPoint(ccp(.5f, .5f));
    m_mainLayer->addChildAtPosition(m_status, Anchor::Bottom, ccp(0.f, kStatusY));
}

void GuidePopup::addEmptyMessage() {
    auto* empty = CCLabelBMFont::create("No macros yet - press Import", kBoldFont);
    if (!empty) return;

    empty->setScale(kEmptyScale);
    empty->setOpacity(170);
    empty->setPosition(ccp(kListWidth / 2.f, kListHeight / 2.f));
    m_scroll->m_contentLayer->addChild(empty);
}

void GuidePopup::addRow(size_t index, float rowTop, bool isLoaded) {
    auto const& entry = m_entries[index];
    float centreY = rowTop - kRowHeight * 0.5f;

    if (index % 2 == 1) {
        auto* stripe = CCLayerColor::create(kRowTint, kListWidth, kRowHeight);
        if (stripe) {
            stripe->setPosition(ccp(0.f, rowTop - kRowHeight));
            m_scroll->m_contentLayer->addChild(stripe);
        }
    }

    auto* name = CCLabelBMFont::create(abbreviate(entry.name, kNameLimit).c_str(), kBoldFont);
    if (name) {
        name->setAnchorPoint(ccp(0.f, .5f));
        name->limitLabelWidth(rowNameWidth(), kRowNameScale, kRowNameMinScale);
        name->setPosition(ccp(kRowLabelInset, centreY));
        name->setColor(isLoaded ? kLoadedColor : ccColor3B{255, 255, 255});
        m_scroll->m_contentLayer->addChild(name);
    }

    auto* menu = CCMenu::create();
    if (!menu) return;
    menu->setPosition(ccp(0.f, 0.f));

    auto* sprite = ButtonSprite::create(isLoaded ? "Loaded" : "Load", "goldFont.fnt",
                                        isLoaded ? "GJ_button_02.png" : "GJ_button_01.png", .8f);
    if (!sprite) return;
    sprite->setScale(.5f);

    auto* button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(GuidePopup::onSelectEntry));
    if (!button) return;

    button->setTag(static_cast<int>(index));
    button->setPosition(ccp(kListWidth - kRowButtonInset, centreY));
    button->setEnabled(!isLoaded);
    menu->addChild(button);

    m_scroll->m_contentLayer->addChild(menu);
}

void GuidePopup::rebuildList() {
    if (!m_scroll || !m_scroll->m_contentLayer) return;

    m_scroll->m_contentLayer->removeAllChildren();
    m_entries = MacroLibrary::list();

    float used = m_entries.size() * (kRowHeight + kRowGap);
    float totalHeight = std::max(kListHeight, used);
    m_scroll->m_contentLayer->setContentSize({kListWidth, totalHeight});

    if (m_entries.empty()) {
        addEmptyMessage();
        m_scroll->moveToTop();
        return;
    }

    auto const& loadedPath = MacroStore::get().path();

    for (size_t index = 0; index < m_entries.size(); ++index) {
        bool isLoaded = !loadedPath.empty() && loadedPath == m_entries[index].path;
        float rowTop = totalHeight - index * (kRowHeight + kRowGap);
        addRow(index, rowTop, isLoaded);
    }

    m_scroll->moveToTop();
}

std::string GuidePopup::alignmentSummary() {
    auto report = Runtime::get().alignmentReport();
    if (report.locked) {
        return fmt::format("lined up ({:+.0f}f)", report.offsetFrames);
    }
    if (report.searching) {
        return fmt::format("lining up {}/{}", report.presses, kMinPressesBeforeLock);
    }
    return "not lined up";
}

void GuidePopup::refreshStatus() {
    if (!m_status) return;

    auto& store = MacroStore::get();
    if (!store.hasMacro()) {
        refreshUnloadButton();
        m_status->setString(m_onlineMessage.empty() ? "No macro loaded" : m_onlineMessage.c_str());
        return;
    }

    refreshUnloadButton();

    if (!m_onlineMessage.empty()) {
        m_status->setString(m_onlineMessage.c_str());
        m_status->limitLabelWidth(kListWidth, kStatusScale, kStatusMinScale);
        return;
    }

    m_status->setString(fmt::format("{}   {} clicks   {}",
                                    abbreviate(store.displayName(), kNameLimit),
                                    store.inputCount(),
                                    alignmentSummary())
                            .c_str());
    m_status->limitLabelWidth(kListWidth, kStatusScale, kStatusMinScale);
}

void GuidePopup::loadMacroAt(std::filesystem::path const& path) {
    std::string error;
    if (!MacroStore::get().loadFromFile(path, error)) {
        FLAlertLayer::create("Load Failed", error, "OK")->show();
        return;
    }

    rememberLoadedMacro(path);
    m_onlineMessage.clear();
    rebuildList();
    refreshStatus();
}

void GuidePopup::unloadMacro() {
    MacroStore::get().clear();
    forgetLoadedMacro();
    Runtime::get().requestRedraw();
    rebuildList();
    refreshStatus();
}

void GuidePopup::onUnload(CCObject*) {
    if (!MacroStore::get().hasMacro()) return;
    unloadMacro();
}

int GuidePopup::currentLevelId() {
    auto* play = PlayLayer::get();
    if (!play || !play->m_level) return 0;
    return play->m_level->m_levelID.value();
}

void GuidePopup::startOnlineSearch() {
    if (m_onlineBusy) return;

    int levelId = currentLevelId();
    if (levelId <= 0) {
        m_onlineMessage = "Open this from a level to search online";
        refreshStatus();
        return;
    }

    m_onlineBusy = true;
    m_onlineMessage = "Asking Hyperbolus...";
    refreshStatus();

    WeakRef<GuidePopup> self = this;
    browseMacrosForLevel(levelId, [self](BrowseResult result) mutable {
        auto popup = self.lock();
        if (!popup) return;

        popup->m_onlineBusy = false;
        if (!result.ok) {
            popup->m_onlineMessage = result.error;
            popup->refreshStatus();
            return;
        }

        popup->m_online = std::move(result.macros);
        if (popup->m_online.empty()) {
            popup->m_onlineMessage = "Hyperbolus has nothing this mod can read for this level";
        } else {
            popup->m_onlineMessage =
                fmt::format("{} usable of {} on Hyperbolus", popup->m_online.size(), result.total);
        }
        popup->showOnlineResults();
        popup->refreshStatus();
    });
}

void GuidePopup::showOnlineResults() {
    if (m_online.empty()) return;

    auto* alert = FLAlertLayer::create("Hyperbolus", m_onlineMessage, "OK");
    if (alert) alert->show();

    OnlineMacro best = m_online.front();
    WeakRef<GuidePopup> self = this;

    downloadMacro(best, [self](FetchResult result) mutable {
        auto popup = self.lock();
        if (!popup) return;

        if (!result.ok) {
            popup->m_onlineMessage = result.error;
            popup->refreshStatus();
            return;
        }

        popup->m_onlineMessage = "Downloaded, loading it now";
        popup->loadMacroAt(result.path);
    });
}

void GuidePopup::onOnline(CCObject*) {
    if (!onlineBrowsingAllowed()) {
        FLAlertLayer::create(
            "Online Is Off",
            "Turn on <cy>Find Macros Online</c> in the settings first. It contacts hyperbolus.net.",
            "OK")
            ->show();
        return;
    }
    startOnlineSearch();
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
