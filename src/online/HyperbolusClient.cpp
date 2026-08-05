#include "HyperbolusClient.hpp"

#include "core/Compat.hpp"
#include "settings/Settings.hpp"
#include "ui/MacroLibrary.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>
#include <deque>
#include <fstream>
#include <system_error>
#include <utility>

#if CGV_GEODE_V5
#include <Geode/utils/async.hpp>
#endif

using namespace geode::prelude;

namespace cgv {

namespace {

constexpr int kRequestTimeoutSeconds = 20;
constexpr int kRateWindowSeconds = 60;

std::string userAgent() {
    return fmt::format("ClickGuideVisualizer/{} (+https://github.com/ohiorizzgod67mango-a11y/click-guide-visualizer)",
                       Mod::get()->getVersion().toNonVString());
}

std::deque<std::chrono::steady_clock::time_point>& requestTimes() {
    static std::deque<std::chrono::steady_clock::time_point> times;
    return times;
}

void pruneOldRequests() {
    auto& times = requestTimes();
    auto cutoff = std::chrono::steady_clock::now() - std::chrono::seconds(kRateWindowSeconds);
    while (!times.empty() && times.front() < cutoff) times.pop_front();
}

std::filesystem::path downloadDestination(OnlineMacro const& macro) {
    std::string base = sanitiseFileName(macro.fileName, macro.displayName() + macro.extension());
    if (base.find('.') == std::string::npos) base += macro.extension();

    auto directory = MacroLibrary::directory();
    std::filesystem::path candidate = directory / base;

    std::error_code code;
    if (!std::filesystem::exists(candidate, code)) return candidate;

    std::filesystem::path stem = candidate.stem();
    std::string extension = candidate.extension().string();
    for (int suffix = 2; suffix < 1000; ++suffix) {
        candidate = directory / fmt::format("{}-{}{}", stem.string(), suffix, extension);
        if (!std::filesystem::exists(candidate, code)) return candidate;
    }
    return candidate;
}

bool writeBytes(std::filesystem::path const& path, ByteVector const& bytes, std::string& errorOut) {
    std::error_code code;
    std::filesystem::create_directories(path.parent_path(), code);

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        errorOut = "Could not open the macro file for writing";
        return false;
    }

    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!out) {
        errorOut = "Could not finish writing the macro file";
        return false;
    }
    return true;
}

web::WebRequest baseRequest() {
    web::WebRequest request;
    request.userAgent(userAgent());
    request.timeout(std::chrono::seconds(kRequestTimeoutSeconds));
    request.header("Accept", "application/json");
    return request;
}

std::string describeStatus(int status) {
    if (status == 429) return "Hyperbolus is rate limiting us, wait a minute";
    if (status == 404) return "Hyperbolus has nothing for that level";
    if (status >= 500) return fmt::format("Hyperbolus had a server error ({})", status);
    if (status == 0) return "Could not reach Hyperbolus, check your connection";
    return fmt::format("Hyperbolus replied with status {}", status);
}

} // namespace

bool onlineBrowsingAllowed() {
    return settings().onlineBrowsing;
}

bool rateLimitAllows() {
    pruneOldRequests();
    return static_cast<int>(requestTimes().size()) < kHyperbolusSelfImposedPerMinute;
}

void noteRequestSent() {
    pruneOldRequests();
    requestTimes().push_back(std::chrono::steady_clock::now());
}

int secondsUntilNextRequest() {
    pruneOldRequests();
    auto& times = requestTimes();
    if (static_cast<int>(times.size()) < kHyperbolusSelfImposedPerMinute) return 0;

    auto ready = times.front() + std::chrono::seconds(kRateWindowSeconds);
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(ready - std::chrono::steady_clock::now());
    return static_cast<int>(std::max<long long>(1, remaining.count()));
}

void browseMacrosForLevel(int gdLevelId, BrowseCallback callback) {
    if (!onlineBrowsingAllowed()) {
        callback(BrowseResult{false, "Online browsing is turned off in the settings", {}, 0});
        return;
    }
    if (gdLevelId <= 0) {
        callback(BrowseResult{false, "This level has no online id to search with", {}, 0});
        return;
    }
    if (!rateLimitAllows()) {
        callback(BrowseResult{
            false, fmt::format("Slow down, try again in {}s", secondsUntilNextRequest()), {}, 0});
        return;
    }

    noteRequestSent();
    std::string url = buildLevelQueryUrl(gdLevelId, 1);
    log::info("{} Asking Hyperbolus for level {}", kLogTag, gdLevelId);

    auto handle = [callback = std::move(callback)](web::WebResponse* response) {
        if (!response) {
            callback(BrowseResult{false, "No response from Hyperbolus", {}, 0});
            return;
        }
        if (!response->ok()) {
            callback(BrowseResult{false, describeStatus(response->code()), {}, 0});
            return;
        }

        MacroSearchPage page;
        std::string error;
        if (!parseMacroListJson(response->string().unwrapOr(""), page, error)) {
            callback(BrowseResult{false, error, {}, 0});
            return;
        }

        auto usable = rankForUse(std::move(page.macros));
        log::info("{} Hyperbolus returned {} macros, {} usable", kLogTag, page.total, usable.size());
        callback(BrowseResult{true, "", std::move(usable), page.total});
    };

#if CGV_GEODE_V5
    async::spawn(baseRequest().get(url), [handle](web::WebResponse response) mutable {
        handle(&response);
    });
#else
    static EventListener<web::WebTask> listener;
    listener.bind([handle](web::WebTask::Event* event) mutable {
        if (auto* response = event->getValue()) handle(response);
    });
    listener.setFilter(baseRequest().get(url));
#endif
}

void downloadMacro(OnlineMacro macro, FetchCallback callback) {
    if (!onlineBrowsingAllowed()) {
        callback(FetchResult{false, "Online browsing is turned off in the settings", {}});
        return;
    }
    if (!macro.usable()) {
        callback(FetchResult{false, "That macro is not in a format this mod can read", {}});
        return;
    }
    if (!rateLimitAllows()) {
        callback(FetchResult{
            false, fmt::format("Slow down, try again in {}s", secondsUntilNextRequest()), {}});
        return;
    }

    noteRequestSent();
    MacroLibrary::ensureDirectory();

    auto destination = downloadDestination(macro);
    std::string url = macro.downloadUrl;

    auto handle = [callback = std::move(callback), destination](web::WebResponse* response) {
        if (!response) {
            callback(FetchResult{false, "No response from Hyperbolus", {}});
            return;
        }
        if (!response->ok()) {
            callback(FetchResult{false, describeStatus(response->code()), {}});
            return;
        }

        auto bytes = response->data();
        if (bytes.empty()) {
            callback(FetchResult{false, "Hyperbolus sent an empty file", {}});
            return;
        }
        if (bytes.size() > kMaxMacroDownloadBytes) {
            callback(FetchResult{false, "That macro is too large to be real", {}});
            return;
        }

        std::string error;
        if (!writeBytes(destination, bytes, error)) {
            callback(FetchResult{false, error, {}});
            return;
        }

        log::info("{} Saved {} bytes to {}", kLogTag, bytes.size(), destination.string());
        callback(FetchResult{true, "", destination});
    };

#if CGV_GEODE_V5
    async::spawn(baseRequest().get(url), [handle](web::WebResponse response) mutable {
        handle(&response);
    });
#else
    static EventListener<web::WebTask> listener;
    listener.bind([handle](web::WebTask::Event* event) mutable {
        if (auto* response = event->getValue()) handle(response);
    });
    listener.setFilter(baseRequest().get(url));
#endif
}

} // namespace cgv
