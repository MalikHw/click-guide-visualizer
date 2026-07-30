#include "MacroStore.hpp"

#include "parse/MacroFile.hpp"

namespace cgv {

MacroStore& MacroStore::get() {
    static MacroStore instance;
    return instance;
}

bool MacroStore::loadFromFile(std::filesystem::path const& path, std::string& errorOut) {
    auto result = loadMacroFile(path);

    if (auto* error = std::get_if<std::string>(&result)) {
        m_lastError = *error;
        errorOut = *error;
        return false;
    }

    m_data = std::make_shared<const ReplayData>(std::move(std::get<ReplayData>(result)));
    m_path = path;
    m_lastError.clear();
    ++m_revision;
    return true;
}

void MacroStore::clear() {
    if (!m_data && m_path.empty()) return;
    m_data.reset();
    m_path.clear();
    m_lastError.clear();
    ++m_revision;
}

std::string MacroStore::displayName() const {
    if (!m_data) return "None";
    if (!m_data->sourceName.empty()) return m_data->sourceName;
    if (!m_path.empty()) return m_path.filename().string();
    return "Macro";
}

} // namespace cgv
