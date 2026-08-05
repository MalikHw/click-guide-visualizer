#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace cgv {

constexpr const char* kHyperbolusHost = "https://hyperbolus.net";
constexpr const char* kHyperbolusMacrosPath = "/api/macros";
constexpr int kHyperbolusRequestsPerMinute = 60;
constexpr int kHyperbolusSelfImposedPerMinute = 30;
constexpr size_t kMaxMacroDownloadBytes = 16 * 1024 * 1024;
constexpr size_t kMaxListedResults = 60;

struct OnlineMacro {
    int id = 0;
    int levelId = 0;
    std::string format;
    std::string levelName;
    std::string levelCreator;
    std::string fileName;
    std::string downloadUrl;
    double framerate = 0.0;
    long long bytes = 0;
    int downloads = 0;
    bool approved = false;
    std::string submittedBy;
    std::string notes;

    bool usable() const;
    std::string displayName() const;
    std::string extension() const;
};

struct MacroSearchPage {
    std::vector<OnlineMacro> macros;
    int currentPage = 1;
    int lastPage = 1;
    int total = 0;
};

std::string buildLevelQueryUrl(int gdLevelId, int page);
bool parseMacroListJson(std::string const& body, MacroSearchPage& out, std::string& errorOut);
std::vector<OnlineMacro> rankForUse(std::vector<OnlineMacro> macros);
std::string sanitiseFileName(std::string const& raw, std::string const& fallback);

} // namespace cgv
