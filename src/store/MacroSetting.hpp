#pragma once

#include <filesystem>
#include <string>

namespace cgv {

constexpr const char* kMacroFileSettingKey = "macro-file";
constexpr const char* kLastMacroSavedKey = "last-macro";

bool settingPathIsPlaceholder(std::filesystem::path const& path);
std::filesystem::path macroFileSettingPath();
void rememberLoadedMacro(std::filesystem::path const& path);
bool loadMacroFromSetting(std::string& errorOut);
bool loadRememberedMacro(std::string& errorOut);

} // namespace cgv
