#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cgv {

struct LibraryEntry {
    std::filesystem::path path;
    std::string name;
};

class MacroLibrary {
public:
    static std::filesystem::path directory();
    static bool ensureDirectory();
    static std::vector<LibraryEntry> list();
    static bool isSupportedExtension(std::filesystem::path const& path);
    static bool importFile(std::filesystem::path const& source, std::filesystem::path& destinationOut,
                           std::string& errorOut);
};

} // namespace cgv
