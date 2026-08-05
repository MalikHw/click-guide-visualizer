#include "HyperbolusModel.hpp"

#include "parse/JsonAccess.hpp"

#include <algorithm>
#include <array>
#include <cctype>

namespace cgv {

namespace {

constexpr std::array<const char*, 4> kSupportedFormats = {"gdr", "gdr2", "mhr", "json"};

int formatRank(std::string const& format) {
    if (format == "gdr2") return 0;
    if (format == "gdr") return 1;
    if (format == "mhr") return 2;
    if (format == "json") return 3;
    return 99;
}

std::string lowercased(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return text;
}

bool isSafeNameCharacter(char character) {
    unsigned char value = static_cast<unsigned char>(character);
    if (std::isalnum(value)) return true;
    return character == '.' || character == '-' || character == '_' || character == ' ';
}

const matjson::Value* firstFile(matjson::Value const& macro) {
    const matjson::Value* files = json::member(macro, "files");
    if (!files) return nullptr;

    auto asArray = files->asArray();
    if (!asArray) return nullptr;

    auto const& entries = asArray.unwrap();
    if (entries.empty()) return nullptr;
    return &entries.front();
}

OnlineMacro readMacro(matjson::Value const& node) {
    OnlineMacro macro;
    macro.id = static_cast<int>(json::numberAt(node, {"id"}, 0));
    macro.levelId = static_cast<int>(json::numberAt(node, {"level_id"}, 0));
    macro.format = lowercased(json::textAt(node, {"format"}, ""));
    macro.framerate = json::numberAt(node, {"fps"}, 0.0);
    macro.notes = json::textAt(node, {"notes"}, "");
    macro.approved = json::member(node, "approved_at") != nullptr &&
                     !json::textAt(node, {"approved_at"}, "").empty();

    if (const matjson::Value* level = json::member(node, "level")) {
        macro.levelName = json::textAt(*level, {"name"}, "");
        macro.levelCreator = json::textAt(*level, {"creator"}, "");
    }

    if (const matjson::Value* author = json::member(node, "author")) {
        macro.submittedBy = json::textAt(*author, {"name", "username"}, "");
    }

    if (const matjson::Value* file = firstFile(node)) {
        macro.fileName = json::textAt(*file, {"filename"}, "");
        macro.downloadUrl = json::textAt(*file, {"url"}, "");
        macro.bytes = static_cast<long long>(json::numberAt(*file, {"bytes"}, 0));
        macro.downloads = static_cast<int>(json::numberAt(*file, {"downloads"}, 0));
    }

    return macro;
}

} // namespace

bool OnlineMacro::usable() const {
    if (downloadUrl.empty()) return false;
    if (downloadUrl.rfind("https://", 0) != 0) return false;
    if (bytes <= 0 || static_cast<size_t>(bytes) > kMaxMacroDownloadBytes) return false;
    return std::find(kSupportedFormats.begin(), kSupportedFormats.end(), format) !=
           kSupportedFormats.end();
}

std::string OnlineMacro::extension() const {
    if (format == "gdr2") return ".gdr2";
    if (format == "mhr") return ".mhr";
    if (format == "json") return ".json";
    return ".gdr";
}

std::string OnlineMacro::displayName() const {
    std::string base = levelName.empty() ? fileName : levelName;
    if (base.empty()) base = "macro";
    return base;
}

std::string buildLevelQueryUrl(int gdLevelId, int page) {
    std::string url = std::string(kHyperbolusHost) + kHyperbolusMacrosPath +
                      "?level_id=" + std::to_string(gdLevelId);
    if (page > 1) url += "&page=" + std::to_string(page);
    return url;
}

bool parseMacroListJson(std::string const& body, MacroSearchPage& out, std::string& errorOut) {
    out = MacroSearchPage{};

    if (body.empty()) {
        errorOut = "Hyperbolus returned an empty response";
        return false;
    }

    auto parsed = matjson::parse(body);
    if (!parsed) {
        errorOut = "Hyperbolus returned something that is not JSON";
        return false;
    }

    matjson::Value root = parsed.unwrap();

    if (const matjson::Value* message = json::member(root, "message")) {
        std::string text = json::toText(*message, "");
        if (!text.empty() && !json::member(root, "data")) {
            errorOut = text;
            return false;
        }
    }

    const matjson::Value* data = json::member(root, "data");
    if (!data) {
        errorOut = "Hyperbolus response had no macro list";
        return false;
    }

    auto asArray = data->asArray();
    if (!asArray) {
        errorOut = "Hyperbolus macro list was not an array";
        return false;
    }

    out.currentPage = static_cast<int>(json::numberAt(root, {"current_page"}, 1));
    out.lastPage = static_cast<int>(json::numberAt(root, {"last_page"}, 1));
    out.total = static_cast<int>(json::numberAt(root, {"total"}, 0));

    for (auto const& node : asArray.unwrap()) {
        if (out.macros.size() >= kMaxListedResults) break;
        out.macros.push_back(readMacro(node));
    }

    return true;
}

std::vector<OnlineMacro> rankForUse(std::vector<OnlineMacro> macros) {
    std::vector<OnlineMacro> usable;
    usable.reserve(macros.size());
    for (auto& macro : macros) {
        if (macro.usable()) usable.push_back(std::move(macro));
    }

    std::stable_sort(usable.begin(), usable.end(), [](OnlineMacro const& a, OnlineMacro const& b) {
        if (a.approved != b.approved) return a.approved;
        int rankA = formatRank(a.format);
        int rankB = formatRank(b.format);
        if (rankA != rankB) return rankA < rankB;
        return a.downloads > b.downloads;
    });

    return usable;
}

std::string sanitiseFileName(std::string const& raw, std::string const& fallback) {
    std::string cleaned;
    cleaned.reserve(raw.size());

    for (char character : raw) {
        if (character == '/' || character == '\\') continue;
        if (isSafeNameCharacter(character)) cleaned.push_back(character);
    }

    while (!cleaned.empty() && (cleaned.front() == ' ' || cleaned.front() == '.')) {
        cleaned.erase(cleaned.begin());
    }
    while (!cleaned.empty() && cleaned.back() == ' ') cleaned.pop_back();

    if (cleaned.size() > 80) cleaned.resize(80);
    if (cleaned.empty()) return fallback;
    return cleaned;
}

} // namespace cgv
