#include "MacroLibrary.hpp"

#include <Geode/Geode.hpp>
#include <algorithm>
#include <array>
#include <cctype>
#include <system_error>

using namespace geode::prelude;

namespace cgv {

namespace {

constexpr std::array<const char*, 4> kSupportedExtensions = {".gdr", ".gdr2", ".json", ".mhr"};

std::string lowercased(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return text;
}

std::filesystem::path uniqueDestination(std::filesystem::path const& directory,
                                        std::filesystem::path const& source) {
    std::filesystem::path candidate = directory / source.filename();
    if (!std::filesystem::exists(candidate)) return candidate;

    std::string stem = source.stem().string();
    std::string extension = source.extension().string();

    for (int suffix = 2; suffix < 1000; ++suffix) {
        candidate = directory / fmt::format("{}-{}{}", stem, suffix, extension);
        if (!std::filesystem::exists(candidate)) return candidate;
    }
    return candidate;
}

} // namespace

std::filesystem::path MacroLibrary::directory() {
    return Mod::get()->getConfigDir() / "macros";
}

bool MacroLibrary::ensureDirectory() {
    std::error_code code;
    std::filesystem::create_directories(directory(), code);
    return !code;
}

bool MacroLibrary::isSupportedExtension(std::filesystem::path const& path) {
    std::string extension = lowercased(path.extension().string());
    return std::find(kSupportedExtensions.begin(), kSupportedExtensions.end(), extension)
        != kSupportedExtensions.end();
}

std::vector<LibraryEntry> MacroLibrary::list() {
    std::vector<LibraryEntry> entries;
    ensureDirectory();

    std::error_code code;
    auto directoryPath = directory();
    if (!std::filesystem::exists(directoryPath, code)) return entries;

    for (auto const& item : std::filesystem::directory_iterator(directoryPath, code)) {
        if (code) break;
        if (!item.is_regular_file(code)) continue;
        if (!isSupportedExtension(item.path())) continue;
        entries.push_back(LibraryEntry{item.path(), item.path().filename().string()});
    }

    std::sort(entries.begin(), entries.end(), [](LibraryEntry const& a, LibraryEntry const& b) {
        return lowercased(a.name) < lowercased(b.name);
    });

    return entries;
}

bool MacroLibrary::importFile(std::filesystem::path const& source, std::filesystem::path& destinationOut,
                              std::string& errorOut) {
    std::error_code code;

    if (!std::filesystem::exists(source, code) || code) {
        errorOut = "Source file does not exist";
        return false;
    }
    if (!ensureDirectory()) {
        errorOut = "Could not create the macro library folder";
        return false;
    }

    auto destination = uniqueDestination(directory(), source);
    std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, code);
    if (code) {
        errorOut = "Could not copy the macro into the library";
        return false;
    }

    destinationOut = destination;
    return true;
}

} // namespace cgv
