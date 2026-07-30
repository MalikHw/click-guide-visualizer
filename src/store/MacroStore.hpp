#pragma once

#include "model/ReplayData.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

namespace cgv {

class MacroStore {
public:
    static MacroStore& get();

    bool loadFromFile(std::filesystem::path const& path, std::string& errorOut);
    void clear();

    bool hasMacro() const { return m_data != nullptr; }
    std::shared_ptr<const ReplayData> data() const { return m_data; }

    uint64_t revision() const { return m_revision; }
    std::filesystem::path const& path() const { return m_path; }
    std::string const& lastError() const { return m_lastError; }

    size_t inputCount() const { return m_data ? m_data->inputs.size() : 0; }
    double framerate() const { return m_data ? m_data->framerate : 0.0; }
    std::string displayName() const;

private:
    MacroStore() = default;

    std::shared_ptr<const ReplayData> m_data;
    std::filesystem::path m_path;
    std::string m_lastError;
    uint64_t m_revision = 0;
};

} // namespace cgv
