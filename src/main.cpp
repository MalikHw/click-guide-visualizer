#include "core/Compat.hpp"
#include "runtime/Hotkeys.hpp"
#include "runtime/Runtime.hpp"
#include "settings/Settings.hpp"
#include "store/MacroSetting.hpp"
#include "store/MacroStore.hpp"
#include "ui/MacroLibrary.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

namespace {

void logLoadedMacro() {
    log::info("{} Loaded macro {} ({} inputs)", cgv::kLogTag,
              cgv::MacroStore::get().displayName(), cgv::MacroStore::get().inputCount());
}

void loadMacroChosenInSettings() {
    std::string error;
    if (cgv::loadMacroFromSetting(error)) {
        logLoadedMacro();
        cgv::Runtime::get().requestRedraw();
        return;
    }
    if (!error.empty()) {
        log::warn("{} Could not load the chosen macro: {}", cgv::kLogTag, error);
    }
}

void applySettingsChange() {
    cgv::SettingsCache::get().refresh();
    loadMacroChosenInSettings();
    cgv::Runtime::get().requestRedraw();
}

void autoLoadLastMacro() {
    if (!cgv::settings().autoLoadMacro) return;

    std::string error;
    if (cgv::loadRememberedMacro(error)) {
        logLoadedMacro();
        return;
    }
    if (!error.empty()) {
        log::warn("{} Could not auto-load macro: {}", cgv::kLogTag, error);
    }
}

void listenForSettingChanges() {
    listenForAllSettingChanges([](std::string_view, std::shared_ptr<SettingV3>) {
        applySettingsChange();
    });
}

} // namespace

$on_mod(Loaded) {
    cgv::SettingsCache::get().refresh();
    cgv::MacroLibrary::ensureDirectory();
    listenForSettingChanges();
    cgv::installHotkeys();
    autoLoadLastMacro();
    loadMacroChosenInSettings();
}
