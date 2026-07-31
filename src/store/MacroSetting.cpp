#include "MacroSetting.hpp"

#include "store/MacroStore.hpp"
#include "ui/MacroLibrary.hpp"

#include <Geode/loader/Mod.hpp>
#include <system_error>

using namespace geode::prelude;

namespace cgv {

namespace {

bool pathPointsAtReadableFile(std::filesystem::path const& path) {
    std::error_code code;
    if (!std::filesystem::exists(path, code) || code) return false;
    return std::filesystem::is_regular_file(path, code) && !code;
}

} // namespace

bool settingPathIsPlaceholder(std::filesystem::path const& path) {
    if (path.empty()) return true;
    if (!path.has_extension()) return true;
    return !MacroLibrary::isSupportedExtension(path);
}

std::filesystem::path macroFileSettingPath() {
    auto* mod = Mod::get();
    if (!mod) return {};
    return mod->getSettingValue<std::filesystem::path>(kMacroFileSettingKey);
}

void rememberLoadedMacro(std::filesystem::path const& path) {
    auto* mod = Mod::get();
    if (!mod) return;
    mod->setSavedValue<std::string>(kLastMacroSavedKey, path.string());
}

bool loadMacroFromSetting(std::string& errorOut) {
    auto path = macroFileSettingPath();
    if (settingPathIsPlaceholder(path)) return false;

    if (MacroStore::get().path() == path && MacroStore::get().hasMacro()) return false;

    if (!pathPointsAtReadableFile(path)) {
        errorOut = "That macro file no longer exists";
        return false;
    }

    if (!MacroStore::get().loadFromFile(path, errorOut)) return false;

    rememberLoadedMacro(path);
    return true;
}

bool loadRememberedMacro(std::string& errorOut) {
    auto* mod = Mod::get();
    if (!mod) return false;

    auto stored = mod->getSavedValue<std::string>(kLastMacroSavedKey);
    if (stored.empty()) return false;

    std::filesystem::path path{stored};
    if (!pathPointsAtReadableFile(path)) {
        errorOut = "The last macro is no longer where it was";
        return false;
    }

    return MacroStore::get().loadFromFile(path, errorOut);
}

} // namespace cgv
