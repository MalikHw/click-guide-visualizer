#pragma once

#include <string>

#include "online/HyperbolusModel.hpp"
#include "ui/MacroLibrary.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <filesystem>
#include <vector>

namespace cgv {

class GuidePopup : public geode::Popup {
public:
    static GuidePopup* create();

protected:
    bool init();

private:
    void buildActionRow();
    void buildStatusLabel();
    void buildScrollList();
    void rebuildList();
    void addEmptyMessage();
    void addRow(size_t index, float rowTop, bool isLoaded);
    static std::string alignmentSummary();
    void refreshStatus();
    void refreshUnloadButton();

    void onImport(cocos2d::CCObject* sender);
    void onFolder(cocos2d::CCObject* sender);
    void onSettings(cocos2d::CCObject* sender);
    void onSelectEntry(cocos2d::CCObject* sender);
    void onUnload(cocos2d::CCObject* sender);
    void onOnline(cocos2d::CCObject* sender);

    void loadMacroAt(std::filesystem::path const& path);
    void unloadMacro();
    void startOnlineSearch();
    void showOnlineResults();
    static int currentLevelId();

    CCMenuItemSpriteExtra* m_unloadButton = nullptr;
    CCMenuItemSpriteExtra* m_onlineButton = nullptr;
    std::vector<OnlineMacro> m_online;
    bool m_onlineBusy = false;
    std::string m_onlineMessage;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    geode::ScrollLayer* m_scroll = nullptr;
    std::vector<LibraryEntry> m_entries;
};

} // namespace cgv
