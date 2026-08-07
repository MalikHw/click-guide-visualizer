#include "MacroCycle.hpp"

#include "store/MacroSetting.hpp"
#include "store/MacroStore.hpp"

namespace cgv {

size_t indexOfPath(std::vector<LibraryEntry> const& entries, std::filesystem::path const& path) {
    if (path.empty()) return kNoMacroIndex;

    for (size_t index = 0; index < entries.size(); ++index) {
        if (entries[index].path == path) return index;
    }
    return kNoMacroIndex;
}

size_t stepIndex(size_t current, size_t count, int direction) {
    if (count == 0) return kNoMacroIndex;

    if (current == kNoMacroIndex) {
        return direction >= 0 ? 0 : count - 1;
    }

    if (direction >= 0) {
        return (current + 1) % count;
    }
    return (current + count - 1) % count;
}

CycleResult cycleMacro(int direction) {
    auto entries = MacroLibrary::list();
    if (entries.empty()) {
        return CycleResult{false, "No macros in your library", {}};
    }

    size_t current = indexOfPath(entries, MacroStore::get().path());
    size_t next = stepIndex(current, entries.size(), direction);
    if (next == kNoMacroIndex) {
        return CycleResult{false, "No macros in your library", {}};
    }

    auto const& chosen = entries[next];

    std::string error;
    if (!MacroStore::get().loadFromFile(chosen.path, error)) {
        return CycleResult{false, error, {}};
    }

    rememberLoadedMacro(chosen.path);
    return CycleResult{true, chosen.name, chosen.path};
}

} // namespace cgv
