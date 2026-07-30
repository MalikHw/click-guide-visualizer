#include "core/Compat.hpp"
#include "runtime/Runtime.hpp"
#include "settings/Settings.hpp"
#include "store/MacroStore.hpp"
#include "ui/MacroLibrary.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

namespace {

void applySettingsChange() {
    cgv::SettingsCache::get().refresh();
    cgv::Runtime::get().requestRedraw();
}

void autoLoadLastMacro() {
    if (!cgv::settings().autoLoadMacro) return;

    auto stored = Mod::get()->getSavedValue<std::string>("last-macro");
    if (stored.empty()) return;

    std::string error;
    if (!cgv::MacroStore::get().loadFromFile(stored, error)) {
        log::warn("{} Could not auto-load macro: {}", cgv::kLogTag, error);
        return;
    }

    log::info("{} Auto-loaded macro {} ({} inputs)", cgv::kLogTag,
              cgv::MacroStore::get().displayName(), cgv::MacroStore::get().inputCount());
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
    autoLoadLastMacro();
}
