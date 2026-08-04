#pragma once

#include <string>

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

    void loadMacroAt(std::filesystem::path const& path);
    void unloadMacro();

    CCMenuItemSpriteExtra* m_unloadButton = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    geode::ScrollLayer* m_scroll = nullptr;
    std::vector<LibraryEntry> m_entries;
};

} // namespace cgv
