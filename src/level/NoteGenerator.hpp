#pragma once

#include "core/Gamemode.hpp"
#include "level/LevelScan.hpp"

#include <cstddef>
#include <vector>

namespace cgv {

struct GeneratedNote {
    float levelX = 0.f;
    float levelY = 0.f;
    float holdSeconds = 0.f;
    Gamemode mode = Gamemode::Cube;
};

struct GenerationInput {
    float startX = 0.f;
    float startY = 0.f;
    float startSpeed = 311.58f;
    Gamemode startMode = Gamemode::Cube;
    float horizonX = 0.f;
};

constexpr float kGenerationStep = 1.f / 240.f;
constexpr size_t kMaxGeneratedNotes = 6000;
constexpr float kFlightSampleUnits = 24.f;

std::vector<GeneratedNote> generateNotes(ScannedLevel const& level, GenerationInput const& input);

} // namespace cgv
