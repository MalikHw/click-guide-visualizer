#pragma once

#include "online/HyperbolusModel.hpp"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace cgv {

struct BrowseResult {
    bool ok = false;
    std::string error;
    std::vector<OnlineMacro> macros;
    int total = 0;
};

struct FetchResult {
    bool ok = false;
    std::string error;
    std::filesystem::path path;
};

using BrowseCallback = std::function<void(BrowseResult)>;
using FetchCallback = std::function<void(FetchResult)>;

bool onlineBrowsingAllowed();
bool rateLimitAllows();
void noteRequestSent();
int secondsUntilNextRequest();

void browseMacrosForLevel(int gdLevelId, BrowseCallback callback);
void downloadMacro(OnlineMacro macro, FetchCallback callback);

} // namespace cgv
