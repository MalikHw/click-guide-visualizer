#pragma once

#include "ui/MacroLibrary.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>

namespace cgv {

constexpr size_t kNoMacroIndex = static_cast<size_t>(-1);

size_t indexOfPath(std::vector<LibraryEntry> const& entries, std::filesystem::path const& path);
size_t stepIndex(size_t current, size_t count, int direction);

struct CycleResult {
    bool ok = false;
    std::string message;
    std::filesystem::path path;
};

CycleResult cycleMacro(int direction);

} // namespace cgv
