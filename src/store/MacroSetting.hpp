#pragma once

#include <filesystem>
#include <string>

namespace cgv {

constexpr const char* kMacroFileSettingKey = "macro-file";
constexpr const char* kLastMacroSavedKey = "last-macro";
constexpr const char* kDismissedMacroSavedKey = "dismissed-macro";

bool settingPathIsPlaceholder(std::filesystem::path const& path);
std::filesystem::path macroFileSettingPath();
void rememberLoadedMacro(std::filesystem::path const& path);
void forgetLoadedMacro();
bool loadMacroFromSetting(std::string& errorOut);
bool loadRememberedMacro(std::string& errorOut);

} // namespace cgv
